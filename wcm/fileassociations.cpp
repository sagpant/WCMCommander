#include <swl.h>
#include "fileassociations.h"
#include "string-util.h"
#include "wcm-config.h"
#include "ltext.h"
#include "strconfig.h"
#include "unicode_lc.h"

static const int CMD_PLUS  = 1000;
static const int CMD_MINUS = 1001;
static const int CMD_EDIT  = 1002;

/// dialog to edit a single file association
class clEditFileAssociationsWin: public NCVertDialog
{
public:
	clEditFileAssociationsWin( NCDialogParent* parent, const clNCFileAssociation* Assoc )
	 : NCVertDialog( ::createDialogAsChild, 0, parent, utf8_to_unicode( _LT( "Edit file associations" ) ).data(), bListOkCancel )
	 , m_Layout( 17, 2 )
	 , m_MaskText( 0, this, utf8_to_unicode( _LT( "A file mask or several file masks (separated with commas)" ) ).data() )
	 , m_MaskEdit( 0, this, 0, 0, 16 )
	 , m_DescriptionText( 0, this, utf8_to_unicode( _LT( "Description of the file association" ) ).data() )
	 , m_DescriptionEdit( 0, this, 0, 0, 16 )
	 , m_ExecuteCommandText( 0, this, utf8_to_unicode( _LT( "Execute command (used for Enter)" ) ).data() )
	 , m_ExecuteCommandEdit( 0, this, 0, 0, 16 )
	 , m_ExecuteCommandSecondaryText( 0, this, utf8_to_unicode( _LT( "Execute command (used for Ctrl+PgDn)" ) ).data() )
	 , m_ExecuteCommandSecondaryEdit( 0, this, 0, 0, 16 )
	 , m_ViewCommandText( 0, this, utf8_to_unicode( _LT( "View command (used for F3)" ) ).data() )
	 , m_ViewCommandEdit( 0, this, 0, 0, 16 )
	 , m_ViewCommandSecondaryText( 0, this, utf8_to_unicode( _LT( "View command (used for Alt+F3)" ) ).data() )
	 , m_ViewCommandSecondaryEdit( 0, this, 0, 0, 16 )
	 , m_EditCommandText( 0, this, utf8_to_unicode( _LT( "Edit command (used for F4)" ) ).data() )
	 , m_EditCommandEdit( 0, this, 0, 0, 16 )
	 , m_EditCommandSecondaryText( 0, this, utf8_to_unicode( _LT( "Edit command (used for Alf+F4)" ) ).data() )
	 , m_EditCommandSecondaryEdit( 0, this, 0, 0, 16 )
	 , m_HasTerminalButton( 0, this, utf8_to_unicode( _LT( "Start in this terminal" ) ).data(), 0, true )
	{
		m_MaskEdit.SetText( utf8_to_unicode( "*" ).data(), true );

		if ( Assoc )
		{
			m_MaskEdit.SetText( Assoc->GetMask().data(), false );
			m_DescriptionEdit.SetText( Assoc->GetDescription().data(), false );
			m_ExecuteCommandEdit.SetText( Assoc->GetExecuteCommand().data(), false );
			m_ExecuteCommandSecondaryEdit.SetText( Assoc->GetExecuteCommandSecondary().data(), false );
			m_ViewCommandEdit.SetText( Assoc->GetViewCommand().data(), false );
			m_ViewCommandSecondaryEdit.SetText( Assoc->GetViewCommandSecondary().data(), false );
			m_EditCommandEdit.SetText( Assoc->GetEditCommand().data(), false );
			m_EditCommandSecondaryEdit.SetText( Assoc->GetEditCommandSecondary().data(), false );
			m_HasTerminalButton.Change( Assoc->GetHasTerminal() );
		}

		m_Layout.AddWinAndEnable( &m_MaskText, 0, 0 );
		m_Layout.AddWinAndEnable( &m_MaskEdit, 1, 0 );
		m_Layout.AddWinAndEnable( &m_DescriptionText, 2, 0 );
		m_Layout.AddWinAndEnable( &m_DescriptionEdit, 3, 0 );
		m_Layout.AddWinAndEnable( &m_HasTerminalButton, 4, 0 );
		m_Layout.AddWinAndEnable( &m_ExecuteCommandText, 5, 0 );
		m_Layout.AddWinAndEnable( &m_ExecuteCommandEdit, 6, 0 );
		m_Layout.AddWinAndEnable( &m_ExecuteCommandSecondaryText, 7, 0 );
		m_Layout.AddWinAndEnable( &m_ExecuteCommandSecondaryEdit, 8, 0 );
		m_Layout.AddWinAndEnable( &m_ViewCommandText, 9, 0 );
		m_Layout.AddWinAndEnable( &m_ViewCommandEdit, 10, 0 );
		m_Layout.AddWinAndEnable( &m_ViewCommandSecondaryText, 11, 0 );
		m_Layout.AddWinAndEnable( &m_ViewCommandSecondaryEdit, 12, 0 );
		m_Layout.AddWinAndEnable( &m_EditCommandText, 13, 0 );
		m_Layout.AddWinAndEnable( &m_EditCommandEdit, 14, 0 );
		m_Layout.AddWinAndEnable( &m_EditCommandSecondaryText, 15, 0 );
		m_Layout.AddWinAndEnable( &m_EditCommandSecondaryEdit, 16, 0 );

		AddLayout( &m_Layout );

		order.append( &m_MaskEdit );
		order.append( &m_DescriptionEdit );
		order.append( &m_ExecuteCommandEdit );
		order.append( &m_ExecuteCommandSecondaryEdit );
		order.append( &m_ViewCommandEdit );
		order.append( &m_ViewCommandSecondaryEdit );
		order.append( &m_EditCommandEdit );
		order.append( &m_EditCommandSecondaryEdit );
		order.append( &m_HasTerminalButton );

		SetPosition();
	}

	std::vector<unicode_t> GetMask() const { return m_MaskEdit.GetText(); }
	std::vector<unicode_t> GetDescription() const { return m_DescriptionEdit.GetText(); }
	std::vector<unicode_t> GetExecuteCommand() const { return m_ExecuteCommandEdit.GetText(); }
	std::vector<unicode_t> GetExecuteCommandSecondary() const { return m_ExecuteCommandSecondaryEdit.GetText(); }
	std::vector<unicode_t> GetViewCommand() const { return m_ViewCommandEdit.GetText(); }
	std::vector<unicode_t> GetViewCommandSecondary() const { return m_ViewCommandSecondaryEdit.GetText(); }
	std::vector<unicode_t> GetEditCommand() const { return m_EditCommandEdit.GetText(); }
	std::vector<unicode_t> GetEditCommandSecondary() const { return m_EditCommandSecondaryEdit.GetText(); }

	const clNCFileAssociation& GetResult() const
	{
		m_Result.SetMask( GetMask() );
		m_Result.SetDescription( GetDescription() );
		m_Result.SetExecuteCommand( GetExecuteCommand() );
		m_Result.SetExecuteCommandSecondary( GetExecuteCommandSecondary() );
		m_Result.SetViewCommand( GetViewCommand() );
		m_Result.SetViewCommandSecondary( GetViewCommandSecondary() );
		m_Result.SetEditCommand( GetEditCommand() );
		m_Result.SetEditCommandSecondary( GetEditCommandSecondary() );
		m_Result.SetHasTerminal( m_HasTerminalButton.IsSet() );

		return m_Result;
	}

private:
	Layout m_Layout;

public:
	StaticLine m_MaskText;
	EditLine   m_MaskEdit;

	StaticLine m_DescriptionText;
	EditLine   m_DescriptionEdit;

	StaticLine m_ExecuteCommandText;
	EditLine   m_ExecuteCommandEdit;

	StaticLine m_ExecuteCommandSecondaryText;
	EditLine   m_ExecuteCommandSecondaryEdit;

	StaticLine m_ViewCommandText;
	EditLine   m_ViewCommandEdit;

	StaticLine m_ViewCommandSecondaryText;
	EditLine   m_ViewCommandSecondaryEdit;

	StaticLine m_EditCommandText;
	EditLine   m_EditCommandEdit;

	StaticLine m_EditCommandSecondaryText;
	EditLine   m_EditCommandSecondaryEdit;

	SButton m_HasTerminalButton;

	mutable clNCFileAssociation m_Result;
};

class clFileAssociationsListWin: public VListWin
{
public:
	clFileAssociationsListWin( Win* parent, std::vector<clNCFileAssociation>* Associations )
	 : VListWin( Win::WT_CHILD, WH_TABFOCUS | WH_CLICKFOCUS, 0, parent, VListWin::SINGLE_SELECT, VListWin::BORDER_3D, 0 )
	 , m_ItemList( Associations )
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
	virtual ~clFileAssociationsListWin();

	const clNCFileAssociation* GetCurrentData() const
	{
		int n = GetCurrent( );
		if ( n < 0 || n >= (int)m_ItemList->size() ) { return NULL; }
		return &( m_ItemList->at(n) );
	}

	clNCFileAssociation* GetCurrentData()
	{
		int n = GetCurrent( );
		if ( n < 0 || n >= (int)m_ItemList->size() ) { return NULL; }
		return &( m_ItemList->at(n) );
	}

	void Ins( const clNCFileAssociation& p )
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

	void Rename( const clNCFileAssociation& p )
	{
		int n = GetCurrent();

		if ( n < 0 || n >= ( int )m_ItemList->size( ) ) { return; }

		m_ItemList->at(n) = p;
		CalcScroll();
		Invalidate();
	}

	virtual void DrawItem( wal::GC& gc, int n, crect rect );

private:
	std::vector<clNCFileAssociation>* m_ItemList;
};

static int uiFcColor = GetUiID( "first-char-color" );

void clFileAssociationsListWin::DrawItem( wal::GC& gc, int n, crect rect )
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

	const clNCFileAssociation* p = &m_ItemList->at(n);

	if ( p )
	{
		gc.Set( GetFont() );
		gc.SetTextColor( color );
		gc.TextOutF( rect.left + 10, rect.top + 1, p->GetMask().data() );
		gc.SetTextColor( fcColor );
		gc.TextOutF( rect.left + 10, rect.top + 1, p->GetMask().data(), 1 );
	}
}

clFileAssociationsListWin::~clFileAssociationsListWin() {};

class clFileAssociationsWin: public NCDialog
{
	clFileAssociationsListWin m_ListWin;
	Layout m_Layout;
	Button m_AddCurrentButton;
	Button m_DelButton;
	Button m_EditButton;

public:
	clFileAssociationsWin( NCDialogParent* parent, std::vector<clNCFileAssociation>* Associations )
	 : NCDialog( ::createDialogAsChild, 0, parent, utf8_to_unicode( _LT( "File associations" ) ).data(), bListOkCancel )
	 , m_Layout( 10, 10 )
	 , m_ListWin( this, Associations )
	 , m_AddCurrentButton( 0, this, utf8_to_unicode( "+ (Ins)" ).data(), CMD_PLUS )
	 , m_DelButton( 0, this, utf8_to_unicode( "- (Del)" ).data(), CMD_MINUS )
	 , m_EditButton( 0, this, utf8_to_unicode( _LT( "Edit" ) ).data(), CMD_EDIT )
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
	void Selected();

	virtual ~clFileAssociationsWin();
};

int uiClassFileAssociations = GetUiID( "Shortcuts" );

int clFileAssociationsWin::UiGetClassId() { return uiClassFileAssociations; }

void clFileAssociationsWin::Selected()
{
	const clNCFileAssociation* p = m_ListWin.GetCurrentData();

	if ( p )
	{
		// we have something selected, tell'em it's ok
		EndModal( CMD_OK );
	}
}

bool clFileAssociationsWin::Command( int id, int subId, Win* win, void* data )
{
	if ( id == CMD_ITEM_CLICK && win == &m_ListWin )
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
		const clNCFileAssociation* p = m_ListWin.GetCurrentData();

		if ( !p ) { return true; }

		if ( NCMessageBox( ( NCDialogParent* )Parent(), _LT( "Delete item" ),
		                   carray_cat<char>( _LT( "Delete '" ), unicode_to_utf8( p->GetMask().data() ).data() , "' ?" ).data(),
		                   false, bListOkCancel ) == CMD_OK )
		{
			m_ListWin.Del();
		}

		return true;
	}

	if ( id == CMD_EDIT )
	{
		const clNCFileAssociation* ValueToEdit = m_ListWin.GetCurrentData();

		if ( !ValueToEdit ) return true;

		clEditFileAssociationsWin Dialog( ( NCDialogParent* )Parent(), ValueToEdit );
		Dialog.SetEnterCmd( 0 );

		if ( Dialog.DoModal( ) == CMD_OK )
		{
			m_ListWin.Rename( Dialog.GetResult( ) );
		}

		return true;
	}

	if ( id == CMD_PLUS )
	{
		clEditFileAssociationsWin Dialog( ( NCDialogParent* )Parent(), NULL );
		Dialog.SetEnterCmd( 0 );

		if ( Dialog.DoModal( ) == CMD_OK )
		{
			m_ListWin.Ins( Dialog.GetResult( ) );
		}

		return true;
	}

	return NCDialog::Command( id, subId, win, data );
}

bool clFileAssociationsWin::Key( cevent_key* pEvent )
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

		if ( pEvent->Key() == VK_F4 )
		{
			Command( CMD_EDIT, 0, this, 0 );
			return true;
		}

		if ( pEvent->Key() == VK_RETURN && m_ListWin.InFocus() )
		{
			Selected();
			return true;
		}

		unicode_t c = UnicodeLC( pEvent->Char() );
	}

	return false;
}

bool clFileAssociationsWin::EventChildKey( Win* child, cevent_key* pEvent )
{
	if ( Key( pEvent ) ) { return true; }

	return NCDialog::EventChildKey( child, pEvent );
}

bool clFileAssociationsWin::EventKey( cevent_key* pEvent )
{
	if ( Key( pEvent ) ) { return true; }

	return NCDialog::EventKey( pEvent );
}

clFileAssociationsWin::~clFileAssociationsWin()
{
}

bool FileAssociationsDlg( NCDialogParent* Parent, std::vector<clNCFileAssociation>* Associations )
{
	// make an editable copy
	std::vector<clNCFileAssociation> LocalAssociations( *Associations );

	clFileAssociationsWin Dialog( Parent, &LocalAssociations );
	Dialog.SetEnterCmd( 0 );

	if ( Dialog.DoModal( ) == CMD_OK )
	{
		// apply changes
		*Associations = LocalAssociations;

		return true;
	}

	return false;
}
