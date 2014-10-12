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

#include <limits.h>

class clColorValidator: public clValidator
{
public:
	virtual bool IsValid( const std::vector<unicode_t>& Str ) const
	{
		if ( Str.size() > 7 ) return false;

		for ( size_t i = 0; i != Str.size(); i++ )
		{
			if ( !Str[i] ) break;

			bool AlphaHex = IsAlphaHex( Str[i] );
			bool Digit = IsDigit( Str[i] );
			bool ValidChar = AlphaHex || Digit;

			if ( !ValidChar ) return false;
		}

		return true;
	}
};

class clColorEditLine: public EditLine
{
public:
	clColorEditLine( int nId, Win* parent, const crect* rect, const unicode_t* txt, int chars = 10, bool frame = true )
	 : EditLine( nId, parent, rect, txt, chars, frame )
	{
		this->SetValidator( new clColorValidator() );
	}
};


/// dialog to edit a single file highlighting rule
class clEditFileHighlightingWin: public NCVertDialog
{
public:
	clEditFileHighlightingWin( NCDialogParent* parent, const clNCFileHighlightingRule* Rule )
	 : NCVertDialog( ::createDialogAsChild, 0, parent, utf8_to_unicode( _LT( "Edit file highlighting" ) ).data(), bListOkCancel )
	 , m_Layout( 17, 2 )
	 , m_MaskText(0, this, utf8_to_unicode(_LT("A file &mask or several file masks (separated with commas)")).data(), &m_MaskEdit)
	 , m_MaskEdit( 0, this, nullptr, nullptr, 16 )
	 , m_DescriptionText(0, this, utf8_to_unicode(_LT("&Description of the file highlighting")).data(), &m_DescriptionEdit)
	 , m_DescriptionEdit( 0, this, nullptr, nullptr, 16 )
	 , m_SizeMinText(0, this, utf8_to_unicode(_LT("Size >= (bytes)")).data(), &m_SizeMinEdit)
	 , m_SizeMinEdit( 0, this, nullptr, nullptr, 16 )
	 , m_SizeMaxText(0, this, utf8_to_unicode(_LT("Size <= (bytes)")).data(), &m_SizeMaxEdit)
	 , m_SizeMaxEdit( 0, this, nullptr, nullptr, 16 )
	 , m_ColorNormalText(0, this, utf8_to_unicode(_LT("Normal color (hexadecimal RGB)")).data(), &m_ColorNormalEdit)
	 , m_ColorNormalEdit(0, this, nullptr, nullptr, 6)
	 , m_HasMaskButton( 0, this, utf8_to_unicode( _LT( "Mask" ) ).data(), 0, true )
	{
		m_MaskEdit.SetText( utf8_to_unicode( "*" ).data(), true );

		clPtr<clUnsignedInt64Validator> Validator = new clUnsignedInt64Validator();

		m_SizeMinEdit.SetValidator( Validator );
		m_SizeMaxEdit.SetValidator( Validator );

		if ( Rule )
		{
			m_MaskEdit.SetText( Rule->GetMask().data(), false );
			m_DescriptionEdit.SetText( Rule->GetDescription().data(), false );
			m_SizeMinEdit.SetText( std::to_wstring( Rule->GetSizeMin() ).c_str(), false );
			m_SizeMaxEdit.SetText( std::to_wstring( Rule->GetSizeMax() ).c_str(), false );
/*
			m_HasTerminalButton.Change( Assoc->GetHasTerminal() );
			m_ExecuteCommandEdit.SetText( Assoc->GetExecuteCommand().data(), false );
			m_ExecuteCommandSecondaryEdit.SetText( Assoc->GetExecuteCommandSecondary().data(), false );
			m_ViewCommandEdit.SetText( Assoc->GetViewCommand().data(), false );
			m_ViewCommandSecondaryEdit.SetText( Assoc->GetViewCommandSecondary().data(), false );
			m_EditCommandEdit.SetText( Assoc->GetEditCommand().data(), false );
			m_EditCommandSecondaryEdit.SetText( Assoc->GetEditCommandSecondary().data(), false );
/*/
		}

		m_Layout.AddWinAndEnable( &m_MaskText, 0, 0 );

		m_Layout.AddWinAndEnable( &m_HasMaskButton, 1, 0 );
		m_Layout.AddWinAndEnable( &m_MaskEdit, 2, 0 );

		m_Layout.AddWinAndEnable( &m_DescriptionText, 3, 0 );
		m_Layout.AddWinAndEnable( &m_DescriptionEdit, 4, 0 );

		m_Layout.AddWinAndEnable( &m_SizeMinText, 5, 0 );
		m_Layout.AddWinAndEnable( &m_SizeMinEdit, 6, 0 );

		m_Layout.AddWinAndEnable( &m_SizeMaxText, 7, 0 );
		m_Layout.AddWinAndEnable( &m_SizeMaxEdit, 8, 0 );

		m_Layout.AddWinAndEnable( &m_ColorNormalText, 9, 0 );
		m_Layout.AddWinAndEnable( &m_ColorNormalEdit, 10, 0 );

		AddLayout( &m_Layout );

		order.append( &m_MaskEdit );
		order.append( &m_HasMaskButton );
		order.append( &m_DescriptionEdit );
		order.append( &m_SizeMinEdit );
		order.append( &m_SizeMaxEdit );
		order.append( &m_ColorNormalEdit );

		SetPosition();
	}

	std::vector<unicode_t> GetMask() const { return m_MaskEdit.GetText(); }
	std::vector<unicode_t> GetDescription() const { return m_DescriptionEdit.GetText(); }
	uint64_t GetSizeMin() const
	{
		std::vector<char> utf8 = unicode_to_utf8( m_SizeMinEdit.GetText().data() );
		uint64_t Result = strtoull( utf8.data(), nullptr, 10 );
		return ( Result == ULLONG_MAX ) ? 0 : Result;
	}
	uint64_t GetSizeMax() const
	{
		std::vector<char> utf8 = unicode_to_utf8( m_SizeMaxEdit.GetText().data() );
		uint64_t Result = strtoull( utf8.data(), nullptr, 10 );
		return ( Result == ULLONG_MAX ) ? 0 : Result;
	}
//	uint64_t GetAttributesMask() const { return strtoull( m_SizeMin.GetText().data(), nullptr, 10); }
/*
	std::vector<unicode_t> GetExecuteCommand() const { return m_ExecuteCommandEdit.GetText(); }
	std::vector<unicode_t> GetExecuteCommandSecondary() const { return m_ExecuteCommandSecondaryEdit.GetText(); }
	std::vector<unicode_t> GetViewCommand() const { return m_ViewCommandEdit.GetText(); }
	std::vector<unicode_t> GetViewCommandSecondary() const { return m_ViewCommandSecondaryEdit.GetText(); }
	std::vector<unicode_t> GetEditCommand() const { return m_EditCommandEdit.GetText(); }
	std::vector<unicode_t> GetEditCommandSecondary() const { return m_EditCommandSecondaryEdit.GetText(); }
*/
	const clNCFileHighlightingRule& GetResult() const
	{
		m_Result.SetMask( GetMask() );
		m_Result.SetDescription( GetDescription() );
		m_Result.SetSizeMin( GetSizeMin() );
		m_Result.SetSizeMax( GetSizeMax() );
/*
		m_Result.SetExecuteCommand( GetExecuteCommand() );
		m_Result.SetExecuteCommandSecondary( GetExecuteCommandSecondary() );
		m_Result.SetViewCommand( GetViewCommand() );
		m_Result.SetViewCommandSecondary( GetViewCommandSecondary() );
		m_Result.SetEditCommand( GetEditCommand() );
		m_Result.SetEditCommandSecondary( GetEditCommandSecondary() );
		m_Result.SetHasTerminal( m_HasTerminalButton.IsSet() );
			*/
		return m_Result;
	}

private:
	Layout m_Layout;

public:
	StaticLabel m_MaskText;
	EditLine   m_MaskEdit;

	StaticLabel m_DescriptionText;
	EditLine   m_DescriptionEdit;

	StaticLabel m_SizeMinText;
	EditLine   m_SizeMinEdit;

	StaticLabel m_SizeMaxText;
	EditLine   m_SizeMaxEdit;

	StaticLabel m_ColorNormalText;
	clColorEditLine m_ColorNormalEdit;

	SButton m_HasMaskButton;

	mutable clNCFileHighlightingRule m_Result;

	bool m_Saved;
};

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
		//int fontW = ( ts.x / ABCStringLen );
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
	 , m_ListWin( this, Rules )
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

		clEditFileHighlightingWin Dialog( ( NCDialogParent* )Parent(), ValueToEdit );
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
		clEditFileHighlightingWin Dialog( ( NCDialogParent* )Parent(), NULL );
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

		//unicode_t c = UnicodeLC( pEvent->Char() );
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

bool accmultimask( const unicode_t* FileName, const std::vector<unicode_t>& MultiMask );

bool clNCFileHighlightingRule::IsRulePassed( const std::vector<unicode_t>& FileName, uint64_t FileSize, uint64_t Attributes ) const
{
	if ( accmultimask( FileName.data(), m_Mask ) ) return true;

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

