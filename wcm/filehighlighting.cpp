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
#include "strmasks.h"

#include <limits.h>

clNCFileHighlightingRule::clNCFileHighlightingRule()
 : m_Mask()
 , m_Description()
 , m_MaskEnabled( false )
 , m_SizeMin( 0 )
 , m_SizeMax( 0 )
 , m_AttributesMask( 0 )

 , m_ColorNormal( 0xFFFF00 )
 , m_ColorNormalBackground( 0x800000 )

 , m_ColorSelected( 0x00FFFF )
 , m_ColorSelectedBackground( 0x800000 )

 , m_ColorUnderCursorNormal( 0x000000 )
 , m_ColorUnderCursorNormalBackground( 0x808000 )

 , m_ColorUnderCursorSelected( 0x00FFFF )
 , m_ColorUnderCursorSelectedBackground( 0x808000 )
{
}

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

enum eColorEditLineType
{
	eColorEditLineType_Foreground,
	eColorEditLineType_Background,
};

class clColorLabel: public Win
{
public:
	clColorLabel( int nId, Win* Parent, const unicode_t* Text, crect* Rect = nullptr );

	//
	// Win interface
	//
	virtual void OnChangeStyles() override;
	virtual void Paint( wal::GC& gc, const crect& PaintRect ) override;

	//
	// clColorLabel
	//
	virtual void SetColors( uint32_t Bg, uint32_t Fg )
	{
		m_BgColor = Bg;
		m_FgColor = Fg;
	}
	virtual void SetBgColor( uint32_t Bg )
	{
		m_BgColor = Bg;
	}
	virtual void SetFgColor( uint32_t Fg )
	{
		m_FgColor = Fg;
	}
	virtual void SetColor( eColorEditLineType Type, uint32_t Color )
	{
		if ( Type == eColorEditLineType_Foreground ) m_FgColor = Color;
		if ( Type == eColorEditLineType_Background ) m_BgColor = Color;
	}

private:
	uint32_t m_BgColor;
	uint32_t m_FgColor;
	std::vector<unicode_t> m_Text;
};

clColorLabel::clColorLabel( int nId, Win* Parent, const unicode_t* Text, crect* Rect )
: Win( Win::WT_CHILD, 0, Parent, Rect, nId )
 , m_BgColor( 0x800000 )
 , m_FgColor( 0xFFFF00 )
 , m_Text( new_unicode_str( Text ) )
{
	if ( !Rect )
	{
		OnChangeStyles();
	}
}

void clColorLabel::OnChangeStyles()
{
	GC gc( this );
	gc.Set( GetFont() );

	cpoint ts = gc.GetTextExtents( m_Text.data() );

	int w = ( ts.x / ABCStringLen ) * m_Text.size();
	int h = ts.y + 2;

	LSize ls;
	ls.x.minimal = ls.x.ideal = w;
	ls.x.maximal = 16000;
	ls.y.minimal = ls.y.maximal = ls.y.ideal = h;
	SetLSize( ls );
}

void clColorLabel::Paint(wal::GC& gc, const crect& PaintRect)
{
	crect rect = ClientRect();
	gc.SetFillColor( m_BgColor );
	gc.FillRect( rect ); 
	gc.Set( GetFont() );
	gc.SetTextColor( m_FgColor );
	gc.TextOutF( 0, 0, m_Text.data() );	
}

class clColorEditLine: public EditLine
{
public:
	clColorEditLine( int nId, Win* parent, const crect* rect, const unicode_t* txt, int chars = 10, bool frame = true, clColorLabel* Label = nullptr, eColorEditLineType Type = eColorEditLineType_Foreground)
	 : EditLine( nId, parent, rect, txt, chars, frame )
	 , m_Label( Label )
	 , m_Type( Type )
	{
		this->SetValidator( new clColorValidator() );
	}

	uint32_t GetEditColor() const
	{
		int64_t Color = HexStrToInt( GetText().data() );

		// truncate
		return uint32_t( uint64_t(Color) & 0xFFFFFFFF );
	}

	void Notify()
	{
		Changed();
	}

protected:
	virtual void Changed() override
	{
		EditLine::Changed();

		if ( m_Label )
		{
			uint32_t Color = GetEditColor();

			m_Label->SetColor( m_Type, Color );
			m_Label->Invalidate();
		}
	}

private:
	clColorLabel* m_Label;
	eColorEditLineType m_Type;
};

/// dialog to edit a single file highlighting rule
class clEditFileHighlightingWin: public NCVertDialog
{
public:
	clEditFileHighlightingWin( NCDialogParent* parent, const clNCFileHighlightingRule* Rule )
	 : NCVertDialog( ::createDialogAsChild, 0, parent, utf8_to_unicode( _LT( "Edit file highlighting" ) ).data(), bListOkCancel )
	 , m_Layout( 21, 3 )
	 , m_MaskText(0, this, utf8_to_unicode(_LT("A file &mask or several masks (separated with commas)")).data(), &m_MaskEdit)
	 , m_MaskEdit( 0, this, nullptr, nullptr, 16 )
	 , m_DescriptionText(0, this, utf8_to_unicode(_LT("&Description of the file highlighting")).data(), &m_DescriptionEdit)
	 , m_DescriptionEdit( 0, this, nullptr, nullptr, 16 )
	 , m_SizeMinText(0, this, utf8_to_unicode(_LT("Size >= (bytes)")).data(), &m_SizeMinEdit)
	 , m_SizeMinEdit( 0, this, nullptr, nullptr, 16 )
	 , m_SizeMaxText(0, this, utf8_to_unicode(_LT("Size <= (bytes)")).data(), &m_SizeMaxEdit)
	 , m_SizeMaxEdit( 0, this, nullptr, nullptr, 16 )
	 , m_ColorText(0, this, utf8_to_unicode(_LT("Colors (hexadecimal BGR)")).data(), nullptr)
	// normal color
	 , m_ColorNormalFGText( 0, this, utf8_to_unicode(_LT("Normal foreground")).data(), &m_ColorNormalFGEdit )
	 , m_ColorNormalFGEdit( 0, this, nullptr, nullptr, 6, true, &m_ColorNormalLabel, eColorEditLineType_Foreground )
	 , m_ColorNormalBGText( 0, this, utf8_to_unicode(_LT("Normal background")).data(), &m_ColorNormalBGEdit )
	 , m_ColorNormalBGEdit( 0, this, nullptr, nullptr, 6, true, &m_ColorNormalLabel, eColorEditLineType_Background )
	 , m_ColorNormalLabel( 0, this, utf8_to_unicode( _LT("filename.ext") ).data() )
	// selected color
	 , m_ColorSelectedFGText(0, this, utf8_to_unicode(_LT("Selected foreground")).data(), &m_ColorNormalFGEdit )
	 , m_ColorSelectedFGEdit( 0, this, nullptr, nullptr, 6, true, &m_ColorSelectedLabel, eColorEditLineType_Foreground )
	 , m_ColorSelectedBGText( 0, this, utf8_to_unicode(_LT("Selected background")).data(), &m_ColorNormalBGEdit )
	 , m_ColorSelectedBGEdit( 0, this, nullptr, nullptr, 6, true, &m_ColorSelectedLabel, eColorEditLineType_Background )
	 , m_ColorSelectedLabel(0, this, utf8_to_unicode( _LT("filename.ext") ).data() )
	// normal color under cursor
	 , m_ColorNormalUnderCursorFGText( 0, this, utf8_to_unicode(_LT("Cursor foreground")).data(), &m_ColorNormalUnderCursorFGEdit )
	 , m_ColorNormalUnderCursorFGEdit( 0, this, nullptr, nullptr, 6, true, &m_ColorNormalUnderCursorLabel, eColorEditLineType_Foreground )
	 , m_ColorNormalUnderCursorBGText( 0, this, utf8_to_unicode(_LT("Cursor background")).data(), &m_ColorNormalUnderCursorBGEdit )
	 , m_ColorNormalUnderCursorBGEdit( 0, this, nullptr, nullptr, 6, true, &m_ColorNormalUnderCursorLabel, eColorEditLineType_Background )
	 , m_ColorNormalUnderCursorLabel( 0, this, utf8_to_unicode( _LT("filename.ext") ).data() )
	// selected color under cursor
	 , m_ColorSelectedUnderCursorFGText(0, this, utf8_to_unicode(_LT("Selected cursor foreground")).data(), &m_ColorNormalUnderCursorFGEdit )
	 , m_ColorSelectedUnderCursorFGEdit( 0, this, nullptr, nullptr, 6, true, &m_ColorSelectedUnderCursorLabel, eColorEditLineType_Foreground )
	 , m_ColorSelectedUnderCursorBGText( 0, this, utf8_to_unicode(_LT("Selected cursor background")).data(), &m_ColorNormalUnderCursorBGEdit )
	 , m_ColorSelectedUnderCursorBGEdit( 0, this, nullptr, nullptr, 6, true, &m_ColorSelectedUnderCursorLabel, eColorEditLineType_Background )
	 , m_ColorSelectedUnderCursorLabel(0, this, utf8_to_unicode( _LT("filename.ext") ).data() )
	//
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

			const size_t Padding = 6;

			m_ColorNormalFGEdit.SetText( IntToHexStr( Rule->GetColorNormal( ), Padding ).c_str( ) );
			m_ColorNormalBGEdit.SetText( IntToHexStr( Rule->GetColorNormalBackground( ), Padding ).c_str( ) );

			m_ColorSelectedFGEdit.SetText( IntToHexStr( Rule->GetColorSelected( ), Padding ).c_str( ) );
			m_ColorSelectedBGEdit.SetText( IntToHexStr( Rule->GetColorSelectedBackground( ), Padding ).c_str( ) );

			m_ColorNormalUnderCursorFGEdit.SetText( IntToHexStr( Rule->GetColorUnderCursorNormal( ), Padding ).c_str( ) );
			m_ColorNormalUnderCursorBGEdit.SetText( IntToHexStr( Rule->GetColorUnderCursorNormalBackground( ), Padding ).c_str( ) );

			m_ColorSelectedUnderCursorFGEdit.SetText( IntToHexStr( Rule->GetColorUnderCursorSelected( ), Padding ).c_str( ) );
			m_ColorSelectedUnderCursorBGEdit.SetText( IntToHexStr( Rule->GetColorUnderCursorSelectedBackground( ), Padding ).c_str( ) );
		}

		m_ColorNormalLabel.SetColors( 0x800000, 0xFFFF00 );
		m_ColorSelectedLabel.SetColors( 0x800000, 0xFFFF00 );

		m_Layout.AddWinAndEnable( &m_MaskText, 0, 0 );

		m_Layout.AddWinAndEnable( &m_HasMaskButton, 1, 0 );
		m_Layout.AddWinAndEnable( &m_MaskEdit, 2, 0 );

		m_Layout.AddWinAndEnable( &m_DescriptionText, 3, 0 );
		m_Layout.AddWinAndEnable( &m_DescriptionEdit, 4, 0 );

		m_Layout.AddWinAndEnable( &m_SizeMinText, 5, 0 );
		m_Layout.AddWinAndEnable( &m_SizeMinEdit, 6, 0 );

		m_Layout.AddWinAndEnable( &m_SizeMaxText, 7, 0 );
		m_Layout.AddWinAndEnable( &m_SizeMaxEdit, 8, 0 );

		m_Layout.AddWinAndEnable( &m_ColorText, 10, 0 );
		// normal
		m_Layout.AddWinAndEnable( &m_ColorNormalFGText, 12, 0 );
		m_ColorNormalFGEdit.SetReplaceMode( true );
		m_Layout.AddWinAndEnable( &m_ColorNormalFGEdit, 13, 0 );
		m_Layout.AddWinAndEnable( &m_ColorNormalBGText, 12, 1 );
		m_ColorNormalBGEdit.SetReplaceMode( true );
		m_Layout.AddWinAndEnable( &m_ColorNormalBGEdit, 13, 1 );
		m_Layout.AddWinAndEnable( &m_ColorNormalLabel, 13, 2 );
		// selected
		m_Layout.AddWinAndEnable( &m_ColorSelectedFGText, 14, 0 );
		m_ColorSelectedFGEdit.SetReplaceMode( true );
		m_Layout.AddWinAndEnable( &m_ColorSelectedFGEdit, 15, 0 );
		m_Layout.AddWinAndEnable( &m_ColorSelectedBGText, 14, 1 );
		m_ColorSelectedBGEdit.SetReplaceMode( true );
		m_Layout.AddWinAndEnable( &m_ColorSelectedBGEdit, 15, 1 );
		m_Layout.AddWinAndEnable( &m_ColorSelectedLabel, 15, 2 );
		// normal under cursor
		m_Layout.AddWinAndEnable( &m_ColorNormalUnderCursorFGText, 16, 0 );
		m_ColorNormalUnderCursorFGEdit.SetReplaceMode( true );
		m_Layout.AddWinAndEnable( &m_ColorNormalUnderCursorFGEdit, 17, 0 );
		m_Layout.AddWinAndEnable( &m_ColorNormalUnderCursorBGText, 16, 1 );
		m_ColorNormalUnderCursorBGEdit.SetReplaceMode( true );
		m_Layout.AddWinAndEnable( &m_ColorNormalUnderCursorBGEdit, 17, 1 );
		m_Layout.AddWinAndEnable( &m_ColorNormalUnderCursorLabel, 17, 2 );
		// selected under cursor
		m_Layout.AddWinAndEnable( &m_ColorSelectedUnderCursorFGText, 18, 0 );
		m_ColorSelectedUnderCursorFGEdit.SetReplaceMode( true );
		m_Layout.AddWinAndEnable( &m_ColorSelectedUnderCursorFGEdit, 19, 0 );
		m_Layout.AddWinAndEnable( &m_ColorSelectedUnderCursorBGText, 18, 1 );
		m_ColorSelectedUnderCursorBGEdit.SetReplaceMode( true );
		m_Layout.AddWinAndEnable( &m_ColorSelectedUnderCursorBGEdit, 19, 1 );
		m_Layout.AddWinAndEnable( &m_ColorSelectedUnderCursorLabel, 19, 2 );
		//

		AddLayout( &m_Layout );

		order.append( &m_MaskEdit );
		order.append( &m_HasMaskButton );
		order.append( &m_DescriptionEdit );
		order.append( &m_SizeMinEdit );
		order.append( &m_SizeMaxEdit );
		order.append( &m_ColorNormalFGEdit );
		order.append( &m_ColorNormalBGEdit );
		order.append( &m_ColorSelectedFGEdit );
		order.append( &m_ColorSelectedBGEdit );
		order.append( &m_ColorNormalUnderCursorFGEdit );
		order.append( &m_ColorNormalUnderCursorBGEdit );
		order.append( &m_ColorSelectedUnderCursorFGEdit );
		order.append( &m_ColorSelectedUnderCursorBGEdit );

		m_ColorNormalFGEdit.Notify();
		m_ColorNormalBGEdit.Notify();
		m_ColorSelectedFGEdit.Notify();
		m_ColorSelectedBGEdit.Notify();
		m_ColorNormalUnderCursorFGEdit.Notify();
		m_ColorNormalUnderCursorBGEdit.Notify();
		m_ColorSelectedUnderCursorFGEdit.Notify();
		m_ColorSelectedUnderCursorBGEdit.Notify();

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

	const clNCFileHighlightingRule& GetResult() const
	{
		m_Result.SetMask( GetMask() );
		m_Result.SetDescription( GetDescription() );
		m_Result.SetSizeMin( GetSizeMin() );
		m_Result.SetSizeMax( GetSizeMax() );

		m_Result.SetColorNormal( (uint32_t)HexStrToInt( m_ColorNormalFGEdit.GetText().data() ) );
		m_Result.SetColorNormalBackground( ( uint32_t )HexStrToInt( m_ColorNormalBGEdit.GetText( ).data( ) ) );

		m_Result.SetColorSelected( ( uint32_t )HexStrToInt( m_ColorSelectedFGEdit.GetText().data() ) );
		m_Result.SetColorSelectedBackground( ( uint32_t )HexStrToInt( m_ColorSelectedBGEdit.GetText().data() ) );

		m_Result.SetColorUnderCursorNormal( ( uint32_t )HexStrToInt( m_ColorNormalUnderCursorFGEdit.GetText().data() ) );
		m_Result.SetColorUnderCursorNormalBackground( ( uint32_t )HexStrToInt( m_ColorNormalUnderCursorBGEdit.GetText( ).data( ) ) );

		m_Result.SetColorUnderCursorSelected( ( uint32_t )HexStrToInt( m_ColorSelectedUnderCursorFGEdit.GetText().data() ) );
		m_Result.SetColorUnderCursorSelectedBackground( ( uint32_t )HexStrToInt( m_ColorSelectedUnderCursorBGEdit.GetText( ).data( ) ) );

		return m_Result;
	}

private:
	Layout m_Layout;

public:
	StaticLabel m_MaskText;
	EditLine    m_MaskEdit;

	StaticLabel m_DescriptionText;
	EditLine    m_DescriptionEdit;

	StaticLabel m_SizeMinText;
	EditLine    m_SizeMinEdit;

	StaticLabel m_SizeMaxText;
	EditLine    m_SizeMaxEdit;

	StaticLabel     m_ColorText;
	StaticLabel     m_ColorNormalFGText;
	clColorEditLine m_ColorNormalFGEdit;
	StaticLabel     m_ColorNormalBGText;
	clColorEditLine m_ColorNormalBGEdit;
	clColorLabel    m_ColorNormalLabel;

	StaticLabel     m_ColorSelectedFGText;
	clColorEditLine m_ColorSelectedFGEdit;
	StaticLabel     m_ColorSelectedBGText;
	clColorEditLine m_ColorSelectedBGEdit;
	clColorLabel    m_ColorSelectedLabel;

	StaticLabel     m_ColorNormalUnderCursorFGText;
	clColorEditLine m_ColorNormalUnderCursorFGEdit;
	StaticLabel     m_ColorNormalUnderCursorBGText;
	clColorEditLine m_ColorNormalUnderCursorBGEdit;
	clColorLabel    m_ColorNormalUnderCursorLabel;

	StaticLabel     m_ColorSelectedUnderCursorFGText;
	clColorEditLine m_ColorSelectedUnderCursorFGEdit;
	StaticLabel     m_ColorSelectedUnderCursorBGText;
	clColorEditLine m_ColorSelectedUnderCursorBGEdit;
	clColorLabel    m_ColorSelectedUnderCursorLabel;


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

bool clNCFileHighlightingRule::IsRulePassed( const std::vector<unicode_t>& FileName, uint64_t FileSize, uint64_t Attributes ) const
{
	clMultimaskSplitter Splitter( m_Mask );

	if ( Splitter.CheckAndFetchAllMasks( FileName.data() ) ) return true;

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

