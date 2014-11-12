/*
   Copyright (c) by Valery Goryachev (Wal)
*/


#include "swl.h"
#include <string.h>

namespace wal
{

	unicode_t ABCString[] = {'A', 'B', 'C', 0};
	int ABCStringLen = 3;

	static clPtr<cfont> pSysFont;

	static cfont* guiFont = 0;

	static char uiDefaultRules[] =
	   "* {color: 0; background: 0xD8E9EC; focus-frame-color : 0x00A000; button-color: 0xD8E9EC;  }"
	   "*:current-item { color: 0xFFFFFF; background: 0x800000; }"
	   "ScrollBar { button-color: 0xD8E9EC;  }"
	   "EditLine:!enabled { background: 0xD8E9EC; }"
	   "EditLine {color: 0; background: 0xFFFFFF; mark-color: 0xFFFFFF; mark-background : 0x800000; }"
//"Button {color: 0; background: 0xFFFFFF}"
//"ButtonWin { color: 0xFFFFFF; background: 0 }"
//"ButtonWin Button {color: 0; background: 0xB0B000 }"
	   ;

#ifdef _WIN32
	void BaseInit()
	{
		static bool initialized = false;

		if ( initialized ) { return; }

		initialized = true;

		pSysFont = new cfont( ( HFONT )GetStockObject( DEFAULT_GUI_FONT ) );
		guiFont = pSysFont.ptr();

//	winColors[IC_BG] = ::GetSysColor(COLOR_BTNFACE);
//	winColors[IC_FG] = winColors[IC_TEXT] = ::GetSysColor(COLOR_BTNTEXT);
//	winColors[IC_GRAY_TEXT] = ::GetSysColor(COLOR_GRAYTEXT);
		UiReadMem( uiDefaultRules );
	}
#else

	void BaseInit()
	{
		dbg_printf( "BaseInit\n" );
		static bool initialized = false;

		if ( initialized ) { return; }

		initialized = true;
		GC gc( ( Win* )0 );
		pSysFont = new cfont( gc, "fixed", 12, cfont::Normal ); // "helvetica",9);
		guiFont = pSysFont.ptr();
		UiReadMem( uiDefaultRules );
	}

#endif

	static cfont* _SysGetFont( Win* w, int fontId )
	{
		return guiFont;
	}

	cfont* ( *SysGetFont )( Win* w, int fontId ) = _SysGetFont;

	void Draw3DButtonW2( GC& gc, crect r, unsigned bg, bool up )
	{
		static unsigned hp1, lp1;
		static unsigned hp2, lp2;
		static unsigned lastBg = 0;
		static bool initialized = false;

		if ( !initialized || lastBg != bg )
		{
			hp1 = ColorTone( bg, -10 );
			lp1 = ColorTone( bg, -90 );
			hp2 = ColorTone( bg, +150 );
			lp2 = ColorTone( bg, -60 );
			lastBg = bg;
			initialized = true;
		}

		unsigned php1, plp1, php2, plp2;

		if ( up )
		{
			php1 = hp1;
			plp1 = lp1;
			php2 = hp2;
			plp2 = lp2;
		}
		else
		{
			php1 = lp1;
			plp1 = hp1;
			php2 = lp2;
			plp2 = hp2;
		}

		gc.SetLine( plp1 );
		gc.MoveTo( r.right - 1, r.top );
		gc.LineTo( r.right - 1, r.bottom - 1 );
		gc.LineTo( r.left, r.bottom - 1 );
		gc.SetLine( php1 );
		gc.LineTo( r.left, r.top );
		gc.LineTo( r.right - 1, r.top );
		r.Dec();
		gc.MoveTo( r.right - 1, r.top );
		gc.SetLine( php2 );
		gc.LineTo( r.left, r.top );
		gc.LineTo( r.left, r.bottom - 1 );
		gc.SetLine( plp2 );
		gc.LineTo( r.right - 1, r.bottom - 1 );
		gc.LineTo( r.right - 1, r.top );
	}


////////////////////////////////////////// StaticLine

	cpoint StaticTextSize( GC& gc, const unicode_t* s, cfont* font )
	{
		cpoint p( 0, 0 );

		if ( font ) { gc.Set( font ); }

		while ( *s )
		{
			const unicode_t* t = unicode_strchr( s, '\n' );
			int len = ( t ? t - s : unicode_strlen( s ) );

			cpoint c = gc.GetTextExtents( s, len );
			s += len;

			if ( t ) { s++; }

			if ( *s == '\r' ) { s++; }

			p.y += c.y;

			if ( p.x < c.x ) { p.x = c.x; }
		}

		return p;
	}

	void DrawStaticText( GC& gc, int x, int y, const unicode_t* s, cfont* font, bool transparent )
	{
		if ( font ) { gc.Set( font ); }

		while ( *s )
		{
			const unicode_t* t = unicode_strchr( s, '\n' );
			int len = ( t ? t - s : unicode_strlen( s ) );
			gc.TextOutF( x, y, s, len );
			cpoint c = gc.GetTextExtents( s, len );
			s += len;

			if ( t ) { s++; }

			if ( *s == '\r' ) { s++; }

			y += c.y;
		}
	}

	cpoint GetStaticTextExtent( GC& gc, const unicode_t* s, cfont* font )
	{
		cpoint res( 0, 0 ), c;

		if ( font ) { gc.Set( font ); }

		while ( *s )
		{
			const unicode_t* t = unicode_strchr( s, '\n' );
			int len = ( t ? t - s : unicode_strlen( s ) );

			c = gc.GetTextExtents( s, len );
			s += len;

			if ( t ) { s++; }

			if ( *s == '\r' ) { s++; }

			res.y += c.y;

			if ( res.x < c.x )
			{
				res.x = c.x;
			}
		}

		return res;
	}

	int uiClassStatic = GetUiID( "Static" );
	int StaticLine::UiGetClassId() {  return uiClassStatic;}

	StaticLine::StaticLine( int nId, Win* parent, const unicode_t* txt, crect* rect, ALIGN al, int w )
		: Win(Win::WT_CHILD, 0, parent, rect, nId)
		, text( txt ? new_unicode_str(txt) : std::vector<wchar_t>() )
		, align( al )
		, width( w )
	{
		if (!rect) 
		{
			GC gc(this);
			if (w >= 0)
			{
				static unicode_t t[]={'A', 'B', 'C', 0};
				cpoint p = GetStaticTextExtent(gc,t,GetFont());
				p.x = p.y * w;
				SetLSize(LSize(p));
			} else if (txt)
				SetLSize(LSize(GetStaticTextExtent(gc,txt,GetFont())));
			else
				SetLSize(LSize(cpoint(0, 0)));
		}
	}


	void StaticLine::Paint( GC& gc, const crect& paintRect )
	{
		crect rect = ClientRect();
		gc.SetFillColor( UiGetColor( uiBackground, 0, 0, 0xFFFFFF )/*GetColor(0)*/ );
		gc.FillRect( rect ); //CCC

		if ( !text.data() || !text[0] ) return;

		gc.SetTextColor( UiGetColor( uiColor, 0, 0, 0 )/*GetColor(IsEnabled() ? IC_TEXT : IC_GRAY_TEXT)*/ ); //CCC
		gc.Set( GetFont() );

		if (align >= 0)
		{
			cpoint size = gc.GetTextExtents( text.data() );
			if (align) //right	
				DrawStaticText( gc, rect.right - size.x, 0, text.data() );
			else //center
				DrawStaticText( gc, (rect.Width() - size.x) / 2, 0, text.data() );
		} else
			DrawStaticText( gc, 0, 0, text.data() );
	}

//////////////////////////////////// ToolTip

	class TBToolTip: public Win
	{
		std::vector<unicode_t> text;
	public:
		TBToolTip( Win* parent, int x, int y, const unicode_t* s );
		virtual void Paint( wal::GC& gc, const crect& paintRect );
		virtual int UiGetClassId();
		virtual ~TBToolTip();
	};

	int uiClassToolTip = GetUiID( "ToolTip" );

	int TBToolTip::UiGetClassId() {   return uiClassToolTip; }

	TBToolTip::TBToolTip( Win* parent, int x, int y, const unicode_t* s )
		:  Win( Win::WT_POPUP, 0, parent ),
		   text( new_unicode_str( s ) )
	{
		wal::GC gc( this );
		cpoint p = GetStaticTextExtent( gc, s, GetFont() );
		Move( crect( x, y, x + p.x + 4, y + p.y + 2 ) );
	}

	void TBToolTip::Paint( wal::GC& gc, const crect& paintRect )
	{
		gc.SetFillColor( UiGetColor( uiBackground, 0, 0, 0xFFFFFF )/*GetColor(IC_BG)*/ ); //0x80FFFF);
		crect r = ClientRect();
		gc.FillRect( r );
		gc.SetTextColor( UiGetColor( uiColor, 0, 0, 0 )/*GetColor(IC_TEXT)*/ );
		//gc.TextOutF(0,0,text.ptr());
		DrawStaticText( gc, 2, 1, text.data(), GetFont(), false );
	}

	TBToolTip::~TBToolTip() {}

	static clPtr<TBToolTip> tip;


	void ToolTipShow( Win* w, int x, int y, const unicode_t* s )
	{
		ToolTipHide();

		if ( !w ) { return; }

		crect r = w->ScreenRect();
		tip = new TBToolTip( w, r.left + x, r.top + y, s );
		tip->Enable();
		tip->Show( Win::SHOW_INACTIVE );
	}

	void ToolTipShow( Win* w, int x, int y, const char* s )
	{
		ToolTipShow( w, x, y, utf8_to_unicode( s ).data() );
	}

	void ToolTipHide()
	{
		tip = 0;
	}



}; //anmespace wal
