/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * wcm@linderdaum.com
 */

#include "ncdialogs.h"
#include "string-util.h"
#include "ltext.h"
#include "globals.h"

#include <algorithm>

class FontExample: public Win
{
	std::vector<unicode_t> text;
	clPtr<cfont> font;
public:
	FontExample( Win* parent, const unicode_t* txt );
	virtual void Paint( wal::GC& gc, const crect& paintRect );
	void SetFont( clPtr<cfont> p ) { font = p; Invalidate(); }
	clPtr<cfont> GetFont() { return font; }
	virtual ~FontExample();
};


FontExample::FontExample( Win* parent, const unicode_t* txt )
	: Win( Win::WT_CHILD, 0, parent, 0 ), text( new_unicode_str( txt ) )
{
	SetLSize( LSize( cpoint( 500, 70 ) ) );
}

void FontExample::Paint( wal::GC& gc, const crect& paintRect )
{
	crect rect = ClientRect();
	unsigned bg = UiGetColor( uiBackground, 0, 0, 0xFFFFFF );
	unsigned color = UiGetColor( uiColor, 0, 0, 0xFFFFFF );

	gc.SetFillColor( bg );
	gc.FillRect( rect );
	Draw3DButtonW2( gc, rect, bg, false );
	int y = 2;

	if ( font.ptr() )
	{
		gc.Set( font.ptr() );

		gc.SetFillColor( bg );
		gc.SetTextColor( color );
		gc.TextOutF( 2, y, text.data() );

		gc.SetFillColor( color );
		gc.SetTextColor( bg );
		gc.TextOutF( 2, y + gc.GetTextExtents( text.data() ).y, text.data() );
	}
}

FontExample::~FontExample() {}


class FontDialogX: public NCVertDialog
{
	Layout iL;
	std::vector<std::string> origList;
	std::vector<std::string> sortedList;
public:
	TextList textList;
	EditLine filterEdit;
	FontExample example;

	FontDialogX( NCDialogParent* parent, bool fixed );
	virtual bool Command( int id, int subId, Win* win, void* data );
	virtual bool EventChildKey( Win* child, cevent_key* pEvent );
	virtual ~FontDialogX();
};

FontDialogX::~FontDialogX() {}

bool FontDialogX::EventChildKey( Win* child, cevent_key* pEvent )
{
	if ( pEvent->Type() == EV_KEYDOWN )
	{
		if (
		   ( pEvent->Key() == VK_UP || pEvent->Key() == VK_DOWN ) && child == &textList
		)
		{
			return false;
		}
	};

	return NCVertDialog::EventChildKey( child, pEvent );
}


namespace wal
{
	extern Display* display;
};


FontDialogX::FontDialogX( NCDialogParent* parent, bool fixed )
	:  NCVertDialog( ::createDialogAsChild, 0, parent, utf8_to_unicode( _LT( "Select X11 server font" ) ).data(), bListOkCancel ),
	   iL( 16, 3 ),
	   textList( Win::WT_CHILD, WH_TABFOCUS | WH_CLICKFOCUS, 0, this, VListWin::SINGLE_SELECT, VListWin::BORDER_3D, 0 ),
	   filterEdit( 0, this, 0, 0, 16 ),
	   example( this, utf8_to_unicode( "Example text: [{(123)}] <?`!@#$%^&*>" ).data() )
{
	iL.AddWin( &textList, 0, 0 );
	textList.Enable();
	textList.Show();
	iL.AddWin( &filterEdit, 1, 0 );
	filterEdit.Enable();
	filterEdit.Show();
	iL.AddWin( &example, 2, 0 );
	example.Enable();
	example.Show();
	AddLayout( &iL );

	SetEnterCmd( CMD_OK );

	{
		char pattern[0x100];
		const char* fixedStr = fixed ? "fixed" : "*";
		snprintf( pattern, sizeof( pattern ) - 1, "-*-%s-*-*-*-*-*-*-*-*-*-*-iso10646-*", fixedStr );
		int count = 0;
		char** ret = XListFonts( display, pattern, 1024 * 10, &count );

		if ( ret )
		{
			for ( int i = 0; i < count; i++ ) { origList.emplace_back( ret[i] ); }

			for ( size_t i = 0; i < origList.size(); i++ ) { sortedList.emplace_back( origList[i] ); }

			std::sort( sortedList.begin(), sortedList.end(), std::less<std::string>() );
		}

		if ( ret ) { XFreeFontNames( ret ); }
	}

	for ( size_t i = 0; i < sortedList.size(); i++ )
	{
		textList.Append( utf8_to_unicode( sortedList[i].c_str() ).data() );
	}

	//MaximizeIfChild();

	textList.SetHeightRange( LSRange( 10, 20, 20 ) ); //in characters
	textList.SetWidthRange( LSRange( 50, 100, 100 ) ); //in characters
	textList.SetFocus();
	textList.MoveCurrent( 0 );

	order.append( &textList );
	order.append( &filterEdit );

	SetPosition();
}


inline int ToUpperEn( int c )
{
	return ( c >= 'a' && c <= 'z' ) ? c - 'a' + 'A' : c;
}

static bool filter_word( const char* str, const char* mask )
{
	if ( !*mask ) { return true; }

	int u = ToUpperEn( *mask );
	mask++;

	for ( ; *str; str++ )
		if ( ToUpperEn( *str ) == u )
		{
			const char* m = mask;

			for ( const char* s = str + 1; *s && *m; s++, m++ )
				if ( ToUpperEn( *s ) != ToUpperEn( *m ) ) { break; }

			if ( !*m ) { return true; }
		}

	return false;
}



bool FontDialogX::Command( int id, int subId, Win* win, void* data )
{
	if ( win == &textList )
	{
		const unicode_t* s = textList.GetCurrentString();

		if ( s )
		{
			example.SetFont( cfont::New( unicode_to_utf8( s ).data() ) );
		}

		return true;
	}


	if ( id == CMD_EDITLINE_INFO )
	{
		std::vector<unicode_t> a = filterEdit.GetText();

		if ( !a.data() ) { return true; }

		std::string str = unicode_to_utf8( a.data() );
		char* s = (char*)str.data();

		std::vector<std::string > flist;

		while ( *s )
		{
			while ( *s && *s <= ' ' ) { s++; }

			if ( *s )
			{
				char* t = s;

				while ( *t && *t > ' ' ) { t++; }

				if ( *t )
				{
					*( t++ ) = 0;
				}

				flist.emplace_back( s );
				s = t;
			}
		}

		textList.Clear();

		for ( size_t i = 0; i < sortedList.size(); i++ )
		{
			char* s = (char*)sortedList[i].c_str();

			bool ok = true;

			for ( size_t j = 0; j < flist.size() && ok; j++ )
			{
				ok = filter_word( s, flist[j].data() );
			}

			if ( ok )
			{
				textList.Append( utf8_to_unicode( sortedList[i].c_str() ).data() );
			}
		}

		textList.DataRefresh();
		textList.MoveCurrent( 0 );

		return true;
	}

	return NCVertDialog::Command( id, subId, win, data );
}

clPtr<cfont> SelectX11Font( NCDialogParent* parent, bool fixed )
{
	FontDialogX dlg( parent, fixed );

	if ( dlg.DoModal() != CMD_OK ) { return 0; }

	const unicode_t* s = dlg.textList.GetCurrentString();

	if ( !s ) { return 0; }

	return cfont::New( unicode_to_utf8( s ).data() );
}

static const char* ftFontPathList[] =
{
	"/usr/share/wcm/fonts",
//	"/usr/local/share/wcm/fonts",
	"/usr/share/fonts",
	"/usr/local/share/fonts",
	0
};

#include <pwd.h>
#include <grp.h>

#include <sys/types.h>
#include <dirent.h>
#include <sys/time.h>

inline bool ExtIs( const char* name, const char* ucExt )
{
	const char* s = strchr( name, '.' );

	if ( !s ) { return false; }

	s++;

	for ( ; *s && *ucExt; s++, ucExt++ )
		if ( ToUpperEn( *s ) != *ucExt ) { return false; }

	return *ucExt == 0  && *s == 0;
}

struct FileNode
{
	std::vector<char> path;
	time_t mtime;
};

static void ScanFontDir( const char* dirName, ccollect<FileNode>& list, int level = 0 )
{
	if ( level > 3 ) { return; }

	DIR* d = opendir( dirName );

	if ( !d ) { return; }

	try
	{
		struct dirent ent, *pEnt;

		while ( true )
		{
			if ( readdir_r( d, &ent, &pEnt ) ) { goto err; }

			if ( !pEnt ) { break; }

			//skip . and ..
			if ( ent.d_name[0] == '.' && ( !ent.d_name[1] || ( ent.d_name[1] == '.' && !ent.d_name[2] ) ) )
			{
				continue;
			}

			std::vector<char> filePath = carray_cat<char>( dirName, "/", ent.d_name );

			struct stat sb;

			if ( stat( filePath.data(), &sb ) ) { continue; }

			if ( ( sb.st_mode & S_IFMT ) == S_IFDIR )
			{
				ScanFontDir( filePath.data(), list, level + 1 );
			}
			else if ( ( sb.st_mode & S_IFMT ) == S_IFREG )
			{
				if ( ExtIs( ent.d_name, "TTF" ) || ExtIs( ent.d_name, "PFB" ) )
				{
					FileNode node;
					node.path = filePath;
					node.mtime = sb.st_mtime;
					list.append( node );
				}
			}
		};

		closedir( d );

		return;

err:
		closedir( d );

		return;

	}
	catch ( ... )
	{
		closedir( d );
		throw;
	}
}

static void GetFontFileList( ccollect<FileNode>& list )
{
	char* home = getenv( "HOME" );

	if ( home )
	{
		ScanFontDir( carray_cat<char>( home, "/.wcm/fonts" ).data(), list );
	}

	const char** p = ftFontPathList;

	for ( ; *p; p++ ) { ScanFontDir( *p, list ); }
}

#ifdef USEFREETYPE
struct FileInfoNode
{
	clPtr<cfont::FTInfo> info;
	time_t mtime;
};

static std::unordered_map< std::string, FileInfoNode > g_LastFileInfo;



///////////////////////////////////////////////////////////////

class FontDialogFT: public NCVertDialog
{
	Layout iL;
	ccollect<FileNode>* fileList;

	struct Node
	{
		const char* fileName;
		cfont::FTInfo* info;
		bool operator<=( const Node& a ) const { return strcmp( info->name.data(), a.info->name.data() ) <= 0; }
		bool operator<( const Node& a ) const { return strcmp( info->name.data(), a.info->name.data() ) < 0; }
		Node(): fileName( 0 ), info( 0 ) {}
	};
	std::vector<Node> list;
	bool fixed;
	int fontSize;
public:
	TextList textList;
	FontExample example;
	StaticLine filterStatic;
	EditLine filterEdit;
	StaticLine sizeStatic;
	EditLine sizeEdit;

	FontDialogFT( NCDialogParent* parent, bool _fixed, ccollect<FileNode>* flist, const char* currentUri );
	virtual bool Command( int id, int subId, Win* win, void* data );
	virtual bool EventChildKey( Win* child, cevent_key* pEvent );
	clPtr<cfont> GetFont() { return example.GetFont(); }
	void ReloadFiltred( const char* filter );
	virtual ~FontDialogFT();
};

bool FontDialogFT::EventChildKey( Win* child, cevent_key* pEvent )
{
	if ( pEvent->Type() == EV_KEYDOWN )
	{
		if (
		   ( pEvent->Key() == VK_UP || pEvent->Key() == VK_DOWN ) && child == &textList
		)
		{
			return false;
		}
	};

	return NCVertDialog::EventChildKey( child, pEvent );
}


FontDialogFT::~FontDialogFT() {}

static int CheckFontSize( int n )
{
	if ( n < 50 ) { return 50; }

	if ( n > 300 ) { return 300; }

	return n;
}


void FontDialogFT::ReloadFiltred( const char* filter )
{
	std::string filtBuf( filter );
	char* s = (char*)filtBuf.data();

	std::vector<std::string> flist;

	while ( *s )
	{
		while ( *s && *s <= ' ' ) { s++; }

		if ( *s )
		{
			char* t = s;

			while ( *t && *t > ' ' ) { t++; }

			if ( *t )
			{
				*( t++ ) = 0;
			}

			flist.emplace_back( s );
			s = t;
		}
	}

	list.clear();
	textList.Clear();

	cstrhash<bool> fontNameHash; //чтоб не задваивались

	for ( int i = 0; i < fileList->count(); i++ )
	{
		const char* fileName = fileList->get( i ).path.data();

		auto iter = g_LastFileInfo.find( fileName );

		if ( iter == g_LastFileInfo.end() ) { continue; }

		const FileInfoNode& Info = iter->second;

		if ( fixed && !( Info.info->flags & cfont::FTInfo::FIXED_WIDTH ) ) { continue; }

		char buffer[0x100];
		snprintf( buffer, sizeof( buffer ), "%s-%s", Info.info->name.data(), Info.info->styleName.data() );

		if ( fontNameHash.exist( buffer ) ) { continue; }

		fontNameHash[buffer] = true;

		char* s = buffer;

		bool ok = true;

		for ( size_t j = 0; j < flist.size() && ok; j++ )
		{
			ok = filter_word( s, flist[j].data() );
		}

		if ( ok )
		{
			Node node;
			node.fileName = fileName;
			node.info = Info.info.ptr();
			list.push_back( node );
		}
	}

	std::sort( list.begin(), list.end(), std::less<Node>() );

	for ( size_t i = 0; i < list.size(); i++ )
	{
		textList.Append( utf8_to_unicode( carray_cat<char>( list[i].info->name.data(), ", ", list[i].info->styleName.data() ).data() ).data(), i, &list[i] );
	}
}


FontDialogFT::FontDialogFT( NCDialogParent* parent, bool _fixed, ccollect<FileNode>* flist, const char* currentUri )
	:  NCVertDialog( ::createDialogAsChild, 0, parent, utf8_to_unicode( _LT( "Select font" ) ).data(), bListOkCancel ),
	   iL( 16, 2 ),
	   fileList( flist ),
	   fixed( _fixed ),
	   fontSize( 100 ),
	   textList( Win::WT_CHILD, WH_TABFOCUS | WH_CLICKFOCUS, 0, this, VListWin::SINGLE_SELECT, VListWin::BORDER_3D, 0 ),
	   example( this, utf8_to_unicode( "Example text: [{(123)}] <?`!@#$%^&*>" ).data() ),
	   filterStatic( 0, this, utf8_to_unicode( "Filter:" ).data() ),
	   filterEdit( 0, this, 0, 0, 16 ),
	   sizeStatic( 0, this, utf8_to_unicode( _LT( "Size:" ) ).data() ),
	   sizeEdit( 0, this, 0, 0, 16 )
{
	iL.AddWin( &textList,  0, 1 );
	textList.Enable();
	textList.Show();
	iL.AddWin( &filterStatic, 1, 0 );
	filterStatic.Enable();
	filterStatic.Show();
	iL.AddWin( &filterEdit, 1, 1 );
	filterEdit.Enable();
	filterEdit.Show();
	iL.AddWin( &sizeStatic, 2, 0 );
	sizeStatic.Enable();
	sizeStatic.Show();
	iL.AddWin( &sizeEdit, 2, 1 );
	sizeEdit.Enable();
	sizeEdit.Show();

	iL.AddWin( &example, 3, 1 );
	example.Enable();
	example.Show();

	textList.SetHeightRange( LSRange( 10, 20, 20 ) ); //in characters
	textList.SetWidthRange( LSRange( 50, 100, 100 ) ); //in characters

	/* LSize ls = textList.GetLSize();
	   if (ls.x.maximal > 400) ls.x.maximal = 400;
	   if (ls.y.maximal > 200) ls.y.maximal = 200;
	   textList.SetLSize(ls);
	*/
	ReloadFiltred( "" );
	textList.SetFocus();
	textList.MoveCurrent( 0 );

	if ( currentUri )
	{
		const char* uri = currentUri;

		if ( uri[0] == '+' )
		{
			uri++;
			int size = 0;

			for ( ; *uri >= '0' && *uri <= '9'; uri++ )
			{
				size = size * 10 + ( *uri - '0' );
			}

			if ( *uri == ':' )
			{
				uri++;
				int n = -1;

				for ( int i = 0; i < (int)list.size(); i++ )
					if ( !strcmp( uri, list[i].fileName ) )
					{
						n = i;
						break;
					}

				if ( n > 0 )
				{
					textList.MoveCurrent( n );
					fontSize = size;
				}
			}
		}
	}

	char buf[64];
	snprintf( buf, sizeof( buf ), "%i.%01i", fontSize / 10, fontSize % 10 );
	sizeEdit.SetText( utf8_to_unicode( buf ).data() );

	AddLayout( &iL );
	SetEnterCmd( CMD_OK );

	SetPosition();

	order.append( &textList );
	order.append( &filterEdit );
	order.append( &sizeEdit );

	SetPosition();
}


bool FontDialogFT::Command( int id, int subId, Win* win, void* data )
{
	if ( win == &textList )
	{
		Node* s = ( Node* )textList.GetCurrentPtr();

		if ( s && s->fileName )
		{
			int n = textList.GetCurrentInt();
			int fsize = ( n >= 0 && n < (int)list.size() ? CheckFontSize( fontSize ) : 100 );
			example.SetFont( cfont::New( s->fileName, fsize ) );
		}

		return true;
	}

	if ( id == CMD_EDITLINE_INFO && win == &filterEdit )
	{
		std::vector<unicode_t> a = filterEdit.GetText();

		if ( !a.data() ) { return true; }

		ReloadFiltred( unicode_to_utf8( a.data() ).data() );

		textList.DataRefresh();
		textList.MoveCurrent( 0 );

		return true;

	}

	if ( id == CMD_EDITLINE_INFO && win == &sizeEdit )
	{
		std::vector<unicode_t> a = sizeEdit.GetText();

		if ( !a.data() ) { return true; }

		std::string str = unicode_to_utf8( a.data() );
		char* s = (char*)str.data();

		while ( *s && *s <= 32 ) { s++; }

		int num = 0;

		for ( ; *s >= '0' && *s <= '9'; s++ ) { num = num * 10 + ( *s - '0' ); }

		int m = 0;

		if ( *s == '.' || *s == ',' )
		{
			s++;

			if ( *s >= '0' && *s <= '9' ) { m = ( *s - '0' ); }
		}

		num = CheckFontSize( num * 10 + m );

		Node* node = ( Node* )textList.GetCurrentPtr();

		if ( node && node->fileName )
		{
			int n = textList.GetCurrentInt();
			int fsize = ( n >= 0 && n < (int)list.size() ? CheckFontSize( fontSize ) : 100 );

			if ( num != fsize )
			{
				example.SetFont( cfont::New( node->fileName, num ) );
				fontSize = num;
			}
		}

		return true;
	}

	return NCVertDialog::Command( id, subId, win, data );
}


///////////////////////////////////////////////////////////////


clPtr<cfont> SelectFTFont( NCDialogParent* parent, bool fixed, const char* currentUri )
{
	ccollect<FileNode> list;
	GetFontFileList( list );

	for ( int i = 0; i < list.count(); i++ )
	{
		auto iter = g_LastFileInfo.find( list[i].path.data() );

		bool Exist = ( iter != g_LastFileInfo.end() );

		if ( Exist )
		{
			if ( iter->second.mtime == list[i].mtime ) { continue; }
		}

		FileInfoNode node;

		node.info = cfont::GetFTFileInfo( list[i].path.data() );
		if (node.info == nullptr) { continue; }
		node.mtime = list[i].mtime;

		g_LastFileInfo[list[i].path.data()] = node;
	};

	FontDialogFT dlg( parent, fixed, &list, currentUri );

	if ( dlg.DoModal() != CMD_OK ) { return 0; }

	return dlg.GetFont();
}
#else

clPtr<cfont> SelectFTFont( NCDialogParent* parent, bool fixed, const char* currentUri )
{
	return 0;
}

#endif
