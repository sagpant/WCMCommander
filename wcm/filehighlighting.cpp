/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#include "dialog_enums.h"
#include "filehighlighting.h"
#include "string-util.h"
#include "wcm-config.h"
#include "ltext.h"
#include "unicode_lc.h"

class clFileHighlightingListWin: public VListWin
{
public:
	clFileHighlightingListWin( Win* parent, std::vector<clNCFileHighlightingRule>* Rules )
	 : VListWin( Win::WT_CHILD, WH_TABFOCUS | WH_CLICKFOCUS, 0, parent, VListWin::SINGLE_SELECT, VListWin::BORDER_3D, 0 )
	 , m_ItemList( Rules )
	{
		wal::GC gc( this );
		gc.Set( GetFont() );
		cpoint ts = gc.GetTextExtents( ABCString );
		int fontW = ( ts.x / ABCStringLen );
		int fontH = ts.y + 2;

		this->SetItemSize( ( fontH > 16 ? fontH : 16 ) + 1, 100 );

		SetCount( m_ItemList->size() );
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
	virtual ~clFileHighlightingListWin();

	const clNCFileHighlightingRule* GetCurrentData( ) const
	{
		int n = GetCurrent( );
		if ( n < 0 || n >= (int)m_ItemList->size() ) { return NULL; }
		return &( m_ItemList->at(n) );
	}

	clNCFileHighlightingRule* GetCurrentData( )
	{
		int n = GetCurrent( );
		if ( n < 0 || n >= (int)m_ItemList->size() ) { return NULL; }
		return &( m_ItemList->at(n) );
	}

	void Ins( const clNCFileHighlightingRule& p )
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

	void Rename( const clNCFileHighlightingRule& p )
	{
		int n = GetCurrent();

		if ( n < 0 || n >= ( int )m_ItemList->size( ) ) { return; }

		m_ItemList->at(n) = p;
		CalcScroll();
		Invalidate();
	}

	virtual void DrawItem( wal::GC& gc, int n, crect rect );

private:
	std::vector<clNCFileHighlightingRule>* m_ItemList;
};

static int uiFcColor = GetUiID( "first-char-color" );

void clFileHighlightingListWin::DrawItem( wal::GC& gc, int n, crect rect )
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

	const clNCFileHighlightingRule* p = &m_ItemList->at( n );

	if ( p )
	{
		gc.Set( GetFont() );
		gc.SetTextColor( color );
		gc.TextOutF( rect.left + 10, rect.top + 1, p->GetMask().data() );
		gc.SetTextColor( fcColor );
		gc.TextOutF( rect.left + 10, rect.top + 1, p->GetMask().data(), 1 );
	}
}

clFileHighlightingListWin::~clFileHighlightingListWin() {};

class clFileHighlightingWin: public NCDialog
{
	clFileHighlightingListWin m_ListWin;
	Layout m_Layout;
	Button m_AddCurrentButton;
	Button m_DelButton;
	Button m_EditButton;
	bool   m_Saved;
public:
	clFileHighlightingWin( NCDialogParent* parent, std::vector<clNCFileHighlightingRule>* Rules )
	 : NCDialog( ::createDialogAsChild, 0, parent, utf8_to_unicode( _LT( "Files highlighting" ) ).data(), bListOkCancel )
	 , m_Layout( 10, 10 )
	 , m_ListWin( this, Rules )
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

	virtual ~clFileHighlightingWin();
};

int uiClassFileHighlighting = GetUiID( "Shortcuts" );

int clFileHighlightingWin::UiGetClassId() { return uiClassFileHighlighting; }

bool clFileHighlightingWin::Command( int id, int subId, Win* win, void* data )
{
	if ( id == CMD_OK )
	{
		EndModal( CMD_OK );
		return true;
	}

	if ( id == CMD_MINUS )
	{
		const clNCFileHighlightingRule* p = m_ListWin.GetCurrentData();

		if ( !p ) { return true; }

		if ( NCMessageBox( ( NCDialogParent* )Parent(), _LT( "Delete item" ),
		                   carray_cat<char>( _LT( "Delete '" ), unicode_to_utf8( p->GetMask().data() ).data() , "' ?" ).data(),
		                   false, bListOkCancel ) == CMD_OK )
		{
			m_ListWin.Del();
			m_Saved = false;
		}

		return true;
	}

	if ( id == CMD_EDIT || ( id == CMD_ITEM_CLICK && win == &m_ListWin ) )
	{
		const clNCFileHighlightingRule* ValueToEdit = m_ListWin.GetCurrentData( );

		if ( !ValueToEdit ) return true;
		/*
		clFileHighlightingWin Dialog( ( NCDialogParent* )Parent(), ValueToEdit );
		Dialog.SetEnterCmd( 0 );

		if ( Dialog.DoModal( ) == CMD_OK )
		{
			m_ListWin.Rename( Dialog.GetResult( ) );
			m_Saved = false;
		}

		return true;
		*/
	}

	if ( id == CMD_PLUS )
	{
		/*
		clEditFileHighlightingWin Dialog( ( NCDialogParent* )Parent(), NULL );
		Dialog.SetEnterCmd( 0 );

		if ( Dialog.DoModal( ) == CMD_OK )
		{
			m_ListWin.Ins( Dialog.GetResult( ) );
			m_Saved = false;
		}

		return true;
		*/
	}

	return NCDialog::Command( id, subId, win, data );
}

bool clFileHighlightingWin::Key( cevent_key* pEvent )
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
			Command( CMD_EDIT, 0, this, 0 );
			return true;
		}

		unicode_t c = UnicodeLC( pEvent->Char() );
	}

	return false;
}

bool clFileHighlightingWin::EventChildKey( Win* child, cevent_key* pEvent )
{
	if ( Key( pEvent ) ) { return true; }

	return NCDialog::EventChildKey( child, pEvent );
}

bool clFileHighlightingWin::EventKey( cevent_key* pEvent )
{
	if ( Key( pEvent ) ) { return true; }

	return NCDialog::EventKey( pEvent );
}

clFileHighlightingWin::~clFileHighlightingWin()
{
}

bool clNCFileHighlightingRule::IsRulePassed( const std::vector<unicode_t>& FileName, uint64_t FileSize, uint64_t Attributes ) const
{
	return false;
}

bool FileHighlightingDlg( NCDialogParent* Parent, std::vector<clNCFileHighlightingRule>* HighlightingRules )
{
	// make an editable copy
	std::vector<clNCFileHighlightingRule> LocalRules( *HighlightingRules );

	clFileHighlightingWin Dialog( Parent, &LocalRules );
	Dialog.SetEnterCmd( 0 );

	if ( Dialog.DoModal( ) == CMD_OK )
	{
		// apply changes
		*HighlightingRules = LocalRules;

		return true;
	}

	return false;
}

