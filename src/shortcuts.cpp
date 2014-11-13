/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#ifdef _WIN32
#	include <winsock2.h>
#endif


#include <swl.h>
#include "shortcuts.h"
#include "vfs.h"
#include "vfs-ftp.h"
#include "vfs-sftp.h"
#include "vfs-smb.h"
#include "string-util.h"
#include "wcm-config.h"
#include "ltext.h"
#include "unicode_lc.h"

inline int CmpNoCase( const unicode_t* a, const unicode_t* b )
{
	unicode_t au = 0;
	unicode_t bu = 0;

	for ( ; *a; a++, b++ )
	{
		au = UnicodeLC( *a );
		bu = UnicodeLC( *b );

		if ( au != bu ) { break; }
	};

	return ( *a ? ( *b ? ( au < bu ? -1 : ( au == bu ? 0 : 1 ) ) : 1 ) : ( *b ? -1 : 0 ) );
}


static const char shortcursSection[] = "shortcuts";

class SCListWin: public VListWin
{
public:
	struct Node
	{
		std::vector<unicode_t> name;
		clPtr<StrConfig> conf;
		bool operator <= ( const Node& a ) { return CmpNoCase( name.data(), a.name.data() ) <= 0; }
	};

	void Sort();
private:
	ccollect<Node> itemList; //static - TEMP !!!

	void Load()
	{
		itemList.clear();

		std::vector< std::string > list;
		LoadStringList( shortcursSection, list );

		for ( int i = 0; i < (int)list.size(); i++ )
		{
			if ( list[i].data() )
			{
				clPtr<StrConfig> cfg = new StrConfig();
				cfg->Load( list[i].data() );
				const char* name = cfg->GetStrVal( "name" );

				if ( name )
				{
					Node node;
					node.conf = cfg;
					node.name = utf8_to_unicode( name );
					itemList.append( node );
				}
			}
		}

		Sort();

		//void SaveStringList(const char *section, ccollect< std::vector<char> > &list);
	}
public:
	SCListWin( Win* parent )
		:  VListWin( Win::WT_CHILD, WH_TABFOCUS | WH_CLICKFOCUS, 0, parent, VListWin::SINGLE_SELECT, VListWin::BORDER_3D, 0 )
	{
		wal::GC gc( this );
		gc.Set( GetFont() );
		cpoint ts = gc.GetTextExtents( ABCString );
//		int fontW = ( ts.x / ABCStringLen );
		int fontH = ts.y + 2;

		this->SetItemSize( ( fontH > 16 ? fontH : 16 ) + 1, 100 );

		Load();
		SetCount( itemList.count() );
		SetCurrent( 0 );

		LSize ls;
		ls.x.maximal = 10000;
		ls.x.ideal = 500;
		ls.x.minimal = 300;

		ls.y.maximal = 10000;
		ls.y.ideal = 300;
		ls.y.minimal = 200;

		SetLSize( ls );
	}

	Node* GetCurrentData() { int n = GetCurrent(); if ( n < 0 || n >= itemList.count() ) { return 0; } return &( itemList[n] ); }

	void Save();

	void Ins( const unicode_t* name, clPtr<StrConfig> p )
	{
		if ( !name || !*name ) { return; }

		Node node;
		node.name = new_unicode_str( name );
		node.conf = p;

		itemList.append( node );
		this->SetCount( itemList.count() );
		this->SetCurrent( itemList.count() - 1 );
		Save();
		Sort();
		CalcScroll();
		Invalidate();
	}

	void Next( unicode_t ch );

	void Del()
	{
		int n = GetCurrent();

		if ( n < 0 || n >= itemList.count() ) { return; }

		itemList.del( n );
		SetCount( itemList.count() );
		Save();
		Sort();
		CalcScroll();
		Invalidate();
	}

	void Rename( const unicode_t* name )
	{
		int n = GetCurrent();

		if ( !name || n < 0 || n >= itemList.count() ) { return; }

		itemList[n].name = new_unicode_str( name );
		Save();
		Sort();
		CalcScroll();
		Invalidate();
	}

	virtual void DrawItem( wal::GC& gc, int n, crect rect );
	virtual ~SCListWin();
};

void SCListWin::Sort()
{
	int count = itemList.count();

	if ( count <= 0 ) { return; }

	int cur = GetCurrent();

	const unicode_t* sel = ( cur >= 0 && cur < count ) ? itemList[cur].name.data() : 0;

	sort2m<Node>( itemList.ptr(), count );

	if ( sel )
	{
		for ( int i = 0; i < count; i++ )
			if ( !CmpNoCase( sel, itemList[i].name.data() ) )
			{
				SetCurrent( i );
				Invalidate();
				break;
			}
	}
}

inline bool EqFirst( const unicode_t* s, int c )
{
	return s && UnicodeLC( *s ) == c;
}

void SCListWin::Next( unicode_t ch )
{
	unicode_t c = UnicodeLC( ch );
	int i;
//	int n = 0;
	int count = itemList.count();
	int current = GetCurrent();

	for ( i = current + 1; i < count; i++ )
		if ( EqFirst( itemList[i].name.data(), c ) ) { goto t; }

	for ( i = 0; i <= current && i < count; i++ )
		if ( EqFirst( itemList[i].name.data(), c ) ) { goto t; }

	return;
t:
	;
	MoveCurrent( i );

	Invalidate();
}



void SCListWin::Save()
{
	std::vector< std::string > list;

	for ( int i = 0; i < itemList.count(); i++ )
		if ( itemList[i].conf.ptr() && itemList[i].name.data() )
		{
			itemList[i].conf->Set( "name", unicode_to_utf8( itemList[i].name.data() ).data() );
			list.push_back( std::string( itemList[i].conf->GetConfig().data() ) );
		}

	SaveStringList( shortcursSection, list );
}

static int uiFcColor = GetUiID( "first-char-color" );

void SCListWin::DrawItem( wal::GC& gc, int n, crect rect )
{
	if ( n >= 0 && n < itemList.count() )
	{
		UiCondList ucl;

		if ( ( n % 2 ) == 0 ) { ucl.Set( uiOdd, true ); }

		if ( n == this->GetCurrent() ) { ucl.Set( uiCurrentItem, true ); }

		unsigned bg = UiGetColor( uiBackground, uiItem, &ucl, 0xB0B000 );
		unsigned color = UiGetColor( uiColor, uiItem, &ucl, 0 );
		unsigned fcColor = UiGetColor( uiFcColor, uiItem, &ucl, 0xFFFF );

		/*
		unsigned bg = ::g_WcmConfig.whiteStyle ? 0xFFFFFF : 0xB0B000;
		unsigned fg =  0;

		unsigned sBg = InFocus() ? 0x800000 : 0x808080;
		unsigned sFg = 0xFFFFFF;
		*/
		gc.SetFillColor( bg );
		gc.FillRect( rect );

		unicode_t* name = itemList[n].name.data();

		if ( name )
		{
			gc.Set( GetFont() );

			gc.SetTextColor( color );
			gc.TextOutF( rect.left + 10, rect.top + 1, name );
			gc.SetTextColor( fcColor );
			gc.TextOutF( rect.left + 10, rect.top + 1, name, 1 );
		}

	}
	else
	{
		gc.SetFillColor( UiGetColor( uiBackground, 0, 0, 0xB0B000 ) );
		gc.FillRect( rect ); //CCC
	}
}

SCListWin::~SCListWin() {};

#define CMD_PLUS 1000
#define CMD_MINUS 1001
#define CMD_RENAME 1002

class ShortcutWin: public NCDialog
{
	Layout lo;
	SCListWin listWin;
	Button addCurrentButton;
	Button delButton;
	Button renameButton;
	clPtr<FS>* fs;
	FSPath* path;
public:
	SCListWin::Node* retData;

	ShortcutWin( NCDialogParent* parent, clPtr<FS>* fp, FSPath* pPath )
		:  NCDialog( ::createDialogAsChild, 0, parent, utf8_to_unicode( _LT( "Shortcuts" ) ).data(), bListOkCancel ),
		   lo( 10, 10 ),
		   listWin( this ),
		   addCurrentButton( 0, this, utf8_to_unicode( "+ (&Ins)" ).data(), CMD_PLUS ),
		   delButton( 0, this, utf8_to_unicode( "- (&Del)" ).data(), CMD_MINUS ),
		   renameButton( 0, this, utf8_to_unicode( _LT( "&Rename" ) ).data(), CMD_RENAME ),
		   fs( fp ),
		   path( pPath ),
		   retData( 0 )
	{
		listWin.Show();
		listWin.Enable();
		addCurrentButton.Show();
		addCurrentButton.Enable();

		delButton.Show();
		delButton.Enable();
		renameButton.Show();
		renameButton.Enable();

		LSize lsize = addCurrentButton.GetLSize();
		LSize lsize2 = delButton.GetLSize();
		LSize lsize3 = renameButton.GetLSize();

		if ( lsize.x.minimal < lsize2.x.minimal ) { lsize.x.minimal = lsize2.x.minimal; }

		if ( lsize.x.minimal < lsize3.x.minimal ) { lsize.x.minimal = lsize3.x.minimal; }

		if ( lsize.x.maximal < lsize.x.minimal ) { lsize.x.maximal = lsize.x.minimal; }

		addCurrentButton.SetLSize( lsize );
		delButton.SetLSize( lsize );
		renameButton.SetLSize( lsize );

		lo.AddWin( &listWin, 0, 0, 9, 0 );

		lo.AddWin( &addCurrentButton, 0, 1 );
		lo.AddWin( &delButton, 1, 1 );
		lo.AddWin( &renameButton, 2, 1 );
		lo.SetLineGrowth( 9 );

		this->AddLayout( &lo );
		//MaximizeIfChild();

		SetPosition();

		listWin.SetFocus();
		this->SetEnterCmd( CMD_OK );
	}

	virtual bool Command( int id, int subId, Win* win, void* data );

	bool Key( cevent_key* pEvent );
	virtual int UiGetClassId();
	virtual bool EventChildKey( Win* child, cevent_key* pEvent );
	virtual bool EventKey( cevent_key* pEvent );

	void Selected();

	virtual ~ShortcutWin();
};

int uiClassShortcut = GetUiID( "Shortcuts" );

int ShortcutWin::UiGetClassId() { return uiClassShortcut; }

void ShortcutWin::Selected()
{
	SCListWin::Node* node = listWin.GetCurrentData();

	if ( node && node->conf.ptr() && node->name.data() )
	{
		retData = node;
		EndModal( CMD_OK );
	}
}

bool ShortcutWin::Command( int id, int subId, Win* win, void* data )
{
	if ( id == CMD_ITEM_CLICK && win == &listWin )
	{
		Selected();
		return true;
	}

	if ( id == CMD_OK )
	{
		Selected();
		return true;
	}

	if ( id == CMD_MINUS )
	{
		SCListWin::Node* node = listWin.GetCurrentData();

		if ( !node || !node->name.data() ) { return true; } //-V560

		if ( NCMessageBox( ( NCDialogParent* )Parent(), _LT( "Delete item" ),
		                   carray_cat<char>( _LT( "Delete '" ), unicode_to_utf8( node->name.data() ).data() , "' ?" ).data(),
		                   false, bListOkCancel ) == CMD_OK )
		{
			listWin.Del();
		}


		return true;
	}

	if ( id == CMD_RENAME )
	{
		SCListWin::Node* node = listWin.GetCurrentData();

		if ( !node || !node->name.data() ) { return true; } //-V560

		std::vector<unicode_t> name = InputStringDialog( ( NCDialogParent* )Parent(), utf8_to_unicode( _LT( "Rename item" ) ).data(),
		                                            node->name.data() );

		if ( name.data() )
		{
			listWin.Rename( name.data() );
		}

		return true;
	}

	if ( id == CMD_PLUS )
	{
		if ( fs && fs->Ptr() && path )
		{
			clPtr<StrConfig> cfg = new StrConfig();

			if ( fs[0]->Type() == FS::SYSTEM )
			{
				cfg->Set( "TYPE", "SYSTEM" );
				cfg->Set( "Path", path->GetUtf8() );
#ifdef _WIN32
				int disk = ( ( FSSys* )fs[0].Ptr() )->Drive();

				if ( disk >= 0 ) { cfg->Set( "Disk", disk ); }

#endif
			}
			else if ( fs[0]->Type() == FS::FTP )
			{
				cfg->Set( "TYPE", "FTP" );
				cfg->Set( "Path", path->GetUtf8() );
				FSFtpParam param;
				( ( FSFtp* )fs[0].Ptr() )->GetParam( &param );
				param.GetConf( *cfg.ptr() );
			}
			else

#if defined(LIBSSH_EXIST) || defined(LIBSSH2_EXIST)
				if ( fs[0]->Type() == FS::SFTP )
				{
					cfg->Set( "TYPE", "SFTP" );
					cfg->Set( "Path", path->GetUtf8() );
					FSSftpParam param;
					( ( FSSftp* )fs[0].Ptr() )->GetParam( &param );
					param.GetConf( *cfg.ptr() );
				}
				else
#endif

#ifdef LIBSMBCLIENT_EXIST
					if ( fs[0]->Type() == FS::SAMBA )
					{
						cfg->Set( "TYPE", "SMB" );
						cfg->Set( "Path", path->GetUtf8() );
						FSSmbParam param;
						( ( FSSmb* )fs[0].Ptr() )->GetParam( &param );
						param.GetConf( *cfg.ptr() );
					}
					else
#endif

						return false;

			std::vector<unicode_t> name = InputStringDialog( ( NCDialogParent* )Parent(), utf8_to_unicode( _LT( "Enter shortcut name" ) ).data(),
			                                            fs[0]->Uri( *path ).GetUnicode() );

			if ( name.data() )
			{
				listWin.Ins( name.data(), cfg );
			}

		}

		return true;
	}

	return NCDialog::Command( id, subId, win, data );
}

bool ShortcutWin::Key( cevent_key* pEvent )
{
	if ( pEvent->Type() == EV_KEYDOWN )
	{
		if ( pEvent->Key() == VK_INSERT )
		{
			Command( CMD_PLUS, 0, this, 0 );
			return true;
		}

		if ( pEvent->Key() == VK_DELETE )
		{
			Command( CMD_MINUS, 0, this, 0 );
			return true;
		}

		if ( pEvent->Key() == VK_RETURN && listWin.InFocus() )
		{
			Selected();
			return true;
		}

		unicode_t c = UnicodeLC( pEvent->Char() );

		if ( c > 32 )
		{
			listWin.Next( c );
			return true;
		}
	}

	return false;
}

bool ShortcutWin::EventChildKey( Win* child, cevent_key* pEvent )
{
	if ( Key( pEvent ) ) { return true; }

	return NCDialog::EventChildKey( child, pEvent );
}

bool ShortcutWin::EventKey( cevent_key* pEvent )
{
	if ( Key( pEvent ) ) { return true; }

	return NCDialog::EventKey( pEvent );
}

ShortcutWin::~ShortcutWin() {}



bool ShortcutDlg( NCDialogParent* parent, clPtr<FS>* fp, FSPath* pPath )
{
	ShortcutWin dlg( parent, fp, pPath );
	dlg.SetEnterCmd( 0 );

	if ( dlg.DoModal() == CMD_OK && dlg.retData && dlg.retData->conf.ptr() && fp && pPath )
	{
		StrConfig* cfg = dlg.retData->conf.ptr();
		const char* type = cfg->GetStrVal( "TYPE" );
		const char* path = cfg->GetStrVal( "PATH" );

		if ( !path ) { return false; }

		if ( !strcmp( type, "SYSTEM" ) )
		{
#ifdef _WIN32
			int drive = cfg->GetIntVal( "DISK" );
			*fp = new FSSys( drive >= 0 ? drive : -1 );
#else
			*fp = new FSSys();
#endif
			pPath->Set( CS_UTF8, path );
			return true;
		}

		if ( !strcmp( type, "FTP" ) )
		{
			FSFtpParam param;
			param.SetConf( *cfg );
			*fp = new FSFtp( &param );
			pPath->Set( CS_UTF8, path );
			return true;
		}

#if defined(LIBSSH_EXIST) || defined(LIBSSH2_EXIST)

		if ( !strcmp( type, "SFTP" ) )
		{
			FSSftpParam param;
			param.SetConf( *cfg );
			*fp = new FSSftp( &param );
			pPath->Set( CS_UTF8, path );
			return true;
		}

#endif

#ifdef LIBSMBCLIENT_EXIST

		if ( !strcmp( type, "SMB" ) )
		{
			FSSmbParam param;
			param.SetConf( *cfg );
			*fp = new FSSmb( &param );
			pPath->Set( CS_UTF8, path );
			return true;
		}

#endif

	}

	return false;
}

