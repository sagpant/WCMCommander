/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * wcm@linderdaum.com
 */

#include <swl.h>
#include "usermenu.h"
#include "string-util.h"
#include "wcm-config.h"
#include "ltext.h"
#include "strconfig.h"
#include "unicode_lc.h"
#include "ncwin.h"

static const int CMD_PLUS  = 1000;
static const int CMD_MINUS = 1001;
static const int CMD_EDIT  = 1002;
static const int CMD_RUN   = 1003;

/// dialog to edit a single file association
class clEditUserMenuWin: public NCVertDialog
{
public:
	clEditUserMenuWin( NCDialogParent* parent, const clNCUserMenuItem* Item )
		: NCVertDialog( ::createDialogAsChild, 0, parent, utf8_to_unicode( _LT( "Edit user menu" ) ).data(), bListOkCancel )
		, m_Layout( 17, 2 )
		, m_DescriptionText( 0, this, utf8_to_unicode( _LT( "&Name. Prepend hotkey with the '&' character" ) ).data(), &m_DescriptionEdit )
		, m_DescriptionEdit( 0, this, 0, 0, 16 )
		, m_CommandText( 0, this, utf8_to_unicode( _LT( "E&xecute command" ) ).data(), &m_CommandEdit )
		, m_CommandEdit( 0, this, 0, 0, 16 )
	{
		if ( Item )
		{
			m_DescriptionEdit.SetText( Item->GetDescription().GetRawText(), false );
			m_CommandEdit.SetText( Item->GetCommand().data(), false );
		}

		m_Layout.AddWinAndEnable( &m_DescriptionText, 2, 0 );
		m_Layout.AddWinAndEnable( &m_DescriptionEdit, 3, 0 );
		m_Layout.AddWinAndEnable( &m_CommandText, 5, 0 );
		m_Layout.AddWinAndEnable( &m_CommandEdit, 6, 0 );
		m_DescriptionEdit.SetFocus();

		AddLayout( &m_Layout );

		order.append( &m_DescriptionEdit );
		order.append( &m_CommandEdit );

		LSize ls;
		ls.x.maximal = 10000;
		ls.x.ideal = 500;
		ls.x.minimal = 500;

		ls.y.maximal = 10000;
		ls.y.ideal = 400;
		ls.y.minimal = 400;

		SetLSize( ls );
		// SetPosition();
	}

	std::vector<unicode_t> GetDescription() const { return m_DescriptionEdit.GetText(); }
	std::vector<unicode_t> GetCommand() const { return m_CommandEdit.GetText(); }

	const clNCUserMenuItem& GetResult() const
	{
		m_Result.SetDescription( GetDescription() );
		m_Result.SetCommand( GetCommand() );
		return m_Result;
	}

private:
	Layout m_Layout;

public:
	StaticLabel m_DescriptionText;
	EditLine   m_DescriptionEdit;

	StaticLabel m_CommandText;
	EditLine   m_CommandEdit;

	mutable clNCUserMenuItem m_Result;

	bool m_Saved;
};

class clUserMenuListWin: public VListWin
{
public:
	clUserMenuListWin( Win* parent, std::vector<clNCUserMenuItem>* Items )
		: VListWin( Win::WT_CHILD, WH_TABFOCUS | WH_CLICKFOCUS, 0, parent, VListWin::SINGLE_SELECT, VListWin::BORDER_3D, 0 )
		, m_ItemList( Items )
	{
		wal::GC gc( this );
		gc.Set( GetFont() );
		cpoint ts = gc.GetTextExtents( ABCString );
		int fontH = ts.y + 2;

		this->SetItemSize( ( fontH > 16 ? fontH : 16 ) + 1, 100 );

		SetCount( m_ItemList->size() );
		SetCurrent( 0 );

		LSize ls;
		ls.x.maximal = 10000;
		ls.x.ideal = 600;
		ls.x.minimal = 600;

		ls.y.maximal = 10000;
		ls.y.ideal = 500;
		ls.y.minimal = 500;

		SetLSize( ls );
	}
	virtual ~clUserMenuListWin();

	const clNCUserMenuItem* GetCurrentData() const
	{
		int n = GetCurrent( );

		if ( n < 0 || n >= ( int )m_ItemList->size() ) { return NULL; }

		return &( m_ItemList->at( n ) );
	}

	clNCUserMenuItem* GetCurrentData( )
	{
		int n = GetCurrent( );

		if ( n < 0 || n >= ( int )m_ItemList->size() ) { return NULL; }

		return &( m_ItemList->at( n ) );
	}

	const clNCUserMenuItem* GetItemMatchingHotkey(unicode_t c) const
	{
		c = UnicodeUC(c);
		for (int i = 0; i < (int)m_ItemList->size(); i++)
		{
			clNCUserMenuItem& item = m_ItemList->at(i);
			if (item.GetDescription().isHotkeyMatching(c))
			{
				return &item;
			}
		}
		return 0;
	}

	

	void Ins( const clNCUserMenuItem& p )
	{
		m_ItemList->push_back( p );
		SetCount( m_ItemList->size( ) );
		SetCurrent( ( int )m_ItemList->size() - 1 );
		CalcScroll();
		Invalidate();
	}

	void Del()
	{
		int n = GetCurrent();

		if ( n < 0 || n >= ( int )m_ItemList->size( ) ) { return; }

		m_ItemList->erase( m_ItemList->begin( ) + n );

		SetCount( m_ItemList->size( ) );
		CalcScroll();
		Invalidate();
	}

	void Rename( const clNCUserMenuItem& p )
	{
		int n = GetCurrent();

		if ( n < 0 || n >= ( int )m_ItemList->size( ) ) { return; }

		m_ItemList->at( n ) = p;
		CalcScroll();
		Invalidate();
	}

	virtual void DrawItem( wal::GC& gc, int n, crect rect );

private:
	std::vector<clNCUserMenuItem>* m_ItemList;
};

static int uiFcColor = GetUiID( "first-char-color" );

void clUserMenuListWin::DrawItem( wal::GC& gc, int n, crect rect )
{
	if ( n < 0 || n >= ( int )m_ItemList->size( ) )
	{
		gc.SetFillColor( UiGetColor( uiBackground, 0, 0, 0xB0B000 ) );
		gc.FillRect( rect ); //CCC
		return;
	}

	UiCondList ucl;

	if ( ( n % 2 ) == 0 ) { ucl.Set( uiOdd, true ); }

	if ( n == this->GetCurrent() ) { ucl.Set( uiCurrentItem, true ); }

	unsigned bg = UiGetColor( uiBackground, uiItem, &ucl, 0xB0B000 );
	unsigned color = UiGetColor( uiColor, uiItem, &ucl, 0 );
	unsigned fcColor = UiGetColor( uiFcColor, uiItem, &ucl, 0xFFFF );

	gc.SetFillColor( bg );
	gc.FillRect( rect );
	gc.Set( GetFont() );

	const clNCUserMenuItem* p = &m_ItemList->at( n );

	if ( p )
	{
		p->GetDescription().DrawItem(gc, rect.left + 10, rect.top + 1, color, fcColor);
	}
}

clUserMenuListWin::~clUserMenuListWin()
{
};

class clUserMenuWin: public NCDialog
{
	clUserMenuListWin m_ListWin;
	Layout m_Layout;
	Button m_AddCurrentButton;
	Button m_DelButton;
	Button m_EditButton;
	bool   m_Saved;
public:
	clUserMenuWin( NCDialogParent* parent, std::vector<clNCUserMenuItem>* Items )
		: NCDialog( ::createDialogAsChild, 0, parent, utf8_to_unicode( _LT( "User menu" ) ).data(), bListOkCancel )
		, m_ListWin( this, Items )
		, m_Layout( 10, 10 )
		, m_AddCurrentButton( 0, this, utf8_to_unicode( "+ (&Ins)" ).data(), CMD_PLUS )
		, m_DelButton( 0, this, utf8_to_unicode( "- (&Del)" ).data(), CMD_MINUS )
		, m_EditButton( 0, this, utf8_to_unicode( _LT( "&Edit" ) ).data(), CMD_EDIT )
		, m_Saved( true )
	{
		m_AddCurrentButton.Enable();
		m_AddCurrentButton.Show();
		m_DelButton.Enable();
		m_DelButton.Show();
		m_EditButton.Enable();
		m_EditButton.Show();

		LSize lsize = m_AddCurrentButton.GetLSize();
		LSize lsize2 = m_DelButton.GetLSize();
		LSize lsize3 = m_EditButton.GetLSize();

		if ( lsize.x.minimal < lsize2.x.minimal ) { lsize.x.minimal = lsize2.x.minimal; }

		if ( lsize.x.minimal < lsize3.x.minimal ) { lsize.x.minimal = lsize3.x.minimal; }

		if ( lsize.x.maximal < lsize.x.minimal ) { lsize.x.maximal = lsize.x.minimal; }

		m_AddCurrentButton.SetLSize( lsize );
		m_DelButton.SetLSize( lsize );
		m_EditButton.SetLSize( lsize );

		m_Layout.AddWinAndEnable( &m_ListWin, 0, 0, 9, 0 );
		m_Layout.AddWin( &m_AddCurrentButton, 0, 1 );
		m_Layout.AddWin( &m_DelButton, 1, 1 );
		m_Layout.AddWin( &m_EditButton, 2, 1 );
		m_Layout.SetLineGrowth( 9 );

		this->AddLayout( &m_Layout );

		SetPosition();

		m_ListWin.SetFocus();
		this->SetEnterCmd( CMD_OK );
	}

	virtual bool Command( int id, int subId, Win* win, void* data );
	virtual int UiGetClassId();
	virtual bool EventChildKey( Win* child, cevent_key* pEvent );
	virtual bool EventKey( cevent_key* pEvent );

	bool Key( cevent_key* pEvent );

	virtual ~clUserMenuWin();
};

int uiClassUserMenu = GetUiID( "Shortcuts" );

int clUserMenuWin::UiGetClassId() { return uiClassUserMenu; }

bool clUserMenuWin::Command( int id, int subId, Win* win, void* data )
{
	if ( id == CMD_OK )
	{
		EndModal( CMD_OK );
		return true;
	}

	if ( id == CMD_RUN )
	{
		EndModal( CMD_RUN );
		NCWin* W = ( NCWin* )Parent();
		const clNCUserMenuItem* ItemToRun =  data ? (const clNCUserMenuItem*) data : m_ListWin.GetCurrentData();

		if ( W && ItemToRun )
		{
			W->StartCommand( ItemToRun->GetCommand(), false, true );
		}

		return true;
	}

	if ( id == CMD_MINUS )
	{
		const clNCUserMenuItem* p = m_ListWin.GetCurrentData();

		if ( !p ) { return true; }

		if ( NCMessageBox( ( NCDialogParent* )Parent(), _LT( "Delete item" ),
		                   carray_cat<char>( _LT( "Delete '" ), unicode_to_utf8( p->GetDescription().GetRawText() ).data() , "'?" ).data(),
		                   false, bListOkCancel ) == CMD_OK )
		{
			m_ListWin.Del();
			m_Saved = false;
		}

		return true;
	}

	if ( id == CMD_EDIT || ( id == CMD_ITEM_CLICK && win == &m_ListWin ) )
	{
		const clNCUserMenuItem* ValueToEdit = m_ListWin.GetCurrentData();

		if ( !ValueToEdit ) { return true; }

		clEditUserMenuWin Dialog( ( NCDialogParent* )Parent(), ValueToEdit );
		Dialog.SetEnterCmd( 0 );

		if ( Dialog.DoModal( ) == CMD_OK )
		{
			m_ListWin.Rename( Dialog.GetResult( ) );
			m_Saved = false;
		}

		return true;
	}

	if ( id == CMD_PLUS )
	{
		clEditUserMenuWin Dialog( ( NCDialogParent* )Parent(), NULL );
		Dialog.SetEnterCmd( 0 );

		if ( Dialog.DoModal( ) == CMD_OK )
		{
			m_ListWin.Ins( Dialog.GetResult( ) );
			m_Saved = false;
		}

		return true;
	}

	return NCDialog::Command( id, subId, win, data );
}

bool clUserMenuWin::Key( cevent_key* pEvent )
{
	if ( pEvent->Type() == EV_KEYDOWN )
	{
		if ( pEvent->Key() == VK_ESCAPE )
		{
			if ( m_Saved || NCMessageBox( ( NCDialogParent* )Parent(), _LT( "Warning" ), _LT( "Quit without saving?" ), true, bListOkCancel ) == CMD_OK )
			{
				return false;
			}

			return true;
		}

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

		if ( pEvent->Key() == VK_F4 )
		{
			Command( CMD_EDIT, 0, this, 0 );
			return true;
		}

		if ( pEvent->Key() == VK_RETURN && m_ListWin.InFocus() )
		{
			if ( m_Saved || NCMessageBox( ( NCDialogParent* )Parent(), _LT( "Warning" ), _LT( "Quit without saving?" ), true, bListOkCancel ) == CMD_OK )
			{
				Command( CMD_RUN, 0, this, 0 );
				return true;
			}
			else
			{
				return false;
			}
		}

		if (( pEvent->Mod() & (KM_ALT | KM_CTRL)) == 0)
		{
			const clNCUserMenuItem* pItem = m_ListWin.GetItemMatchingHotkey(pEvent->Char());

			if (pItem != 0)
			{
				Command(CMD_RUN, 0, this, (void*)pItem);
				return true;
			}
		}
		return false;

	}

	return false;
}

bool clUserMenuWin::EventChildKey( Win* child, cevent_key* pEvent )
{
	if ( Key( pEvent ) ) { return true; }

	return NCDialog::EventChildKey( child, pEvent );
}

bool clUserMenuWin::EventKey( cevent_key* pEvent )
{
	if ( Key( pEvent ) ) { return true; }

	return NCDialog::EventKey( pEvent );
}

clUserMenuWin::~clUserMenuWin()
{
}

bool UserMenuDlg( NCDialogParent* Parent, std::vector<clNCUserMenuItem>* MenuItems )
{
	if ( !MenuItems ) { return false; }

	// make an editable copy
	std::vector<clNCUserMenuItem> LocalItems( *MenuItems );

	clUserMenuWin Dialog( Parent, &LocalItems );
	Dialog.SetEnterCmd( 0 );

	if ( Dialog.DoModal( ) == CMD_OK )
	{
		// apply changes
		*MenuItems = LocalItems;

		return true;
	}

	return false;
}
