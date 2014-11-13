/*
   Copyright (c) by Valery Goryachev (Wal)
*/


#include "swl.h"
// XXX: refactor to move the header to .
#include "../unicode_lc.h" 

#define CBC_BOXFRAME 0
#define CBC_BOX_BG 0xFFFFFF
#define CBC_CHECK 0x008000
//#define CBC_BK (::GetSysColor(COLOR_BTNFACE))

namespace wal
{

	int uiClassSButton = GetUiID( "SButton" );
	int SButton::UiGetClassId() {  return uiClassSButton; }

	static unsigned short chPix[] = {7, 0x40, 0x60, 0x71, 0x3B, 0x1F, 0x0E, 0x04};
	static unsigned short rbPix[] = {5, 0xE, 0x1F, 0x1F, 0x1F, 0xE};

	static void DrawCB( GC& gc, int x, int y, bool isSet )
	{
		gc.SetLine( CBC_BOXFRAME );
		gc.MoveTo( x, y );
		gc.LineTo( x + 13, y );
		gc.LineTo( x + 13, y + 13 );
		gc.LineTo( x, y + 13 );
		gc.LineTo( x, y );
		gc.SetFillColor( CBC_BOX_BG ); //CCC
		gc.FillRect( crect( x + 1, y + 1, x + 13, y + 13 ) );

		if ( isSet )
		{
			DrawPixelList( gc, chPix, x + 3, y + 3, CBC_CHECK );
		}
	}

	static void DrawCE( GC& gc, int x, int y, bool isSet )
	{
		gc.SetLine( CBC_BOXFRAME );
		gc.SetFillColor( CBC_BOX_BG );
		gc.Ellipce( crect( x, y, x + 13, y + 13 ) );

		if ( isSet )
		{
			DrawPixelList( gc, rbPix, x + 4, y + 4, CBC_CHECK );
		}
	}

	void SButton::Change( bool _isSet )
	{
		bool old = isSet;
		isSet = _isSet;

		if ( isSet != old && IsVisible() ) { Invalidate(); }

		if ( group > 0 && IsSet() && Parent() )
		{
			Parent()->SendBroadcast( CMD_SBUTTON_INFO, IsSet() ? SCMD_SBUTTON_CHECKED : SCMD_SBUTTON_CLEARED, this, &isSet, 2 );
		}

		if ( Parent() && isSet != old )
		{
			Parent()->Command( CMD_SBUTTON_INFO, IsSet() ? SCMD_SBUTTON_CHECKED : SCMD_SBUTTON_CLEARED , this, &isSet );
		}
	}

	SButton::SButton( int nId, Win* parent, unicode_t* txt, int _group, bool _isSet, crect* rect )
		:  Win( Win::WT_CHILD, Win::WH_TABFOCUS | WH_CLICKFOCUS, parent, rect, nId ),
		   isSet( _isSet ),
		   text( txt ),
		   group( _group )
	{
		if ( !rect )
		{
			GC gc( this );
			gc.Set( GetFont() );
			cpoint p = gc.GetTextExtents( txt );

			if ( p.y < 16 ) { p.y = 16; }

			p.x += 17 + 4;
			p.y += 2;
			SetLSize( LSize( p ) );
		}

		Change( isSet );
	}

	bool SButton::Broadcast( int id, int subId, Win* win, void* data )
	{
		if ( id == CMD_SBUTTON_INFO && subId == SCMD_SBUTTON_CHECKED
		     && win != this
		     && group && IsSet() && ( ( SButton* )win )->group == group )
		{
			Change( false );
			return true;
		}

		return false;
	}

	void SButton::Paint( GC& gc, const crect& paintRect )
	{
		crect cr = ClientRect();

		unsigned colorBg = UiGetColor( uiBackground, 0, 0, 0xFFFFFF );

		gc.SetFillColor( colorBg ); //CCC
		gc.FillRect( cr );

		if ( group > 0 )
		{
			DrawCE( gc, 1, ( cr.Height() - 13 ) / 2, IsSet() );
		}
		else
		{
			DrawCB( gc, 1, ( cr.Height() - 13 ) / 2, IsSet() );
		}

		gc.Set( GetFont() );
		cpoint tsize = text.GetTextExtents(gc);

		gc.SetFillColor( colorBg );
		//gc.SetTextColor( UiGetColor( uiColor, 0, 0, 0 ) );

		//gc.TextOutF( 14 + 1 + 1 + 1 , ( cr.Height() - tsize.y ) / 2, text.data() );
		UiCondList ucl;
		int color_text = UiGetColor(uiColor, uiClassSButton, &ucl, 0x0);
		int color_hotkey = UiGetColor(uiHotkeyColor, uiClassSButton, &ucl, 0x0);
		text.DrawItem(gc, 14 + 1 + 1 + 1, (cr.Height() - tsize.y) / 2, color_text, color_hotkey);

		if ( InFocus() )
		{
			crect rect;
			rect.left = 14 + 2;
			rect.top = ( cr.Height() - tsize.y - 2 ) / 2;
			rect.right = rect.left + tsize.x + 4;
			rect.bottom = rect.top + tsize.y + 2;
			DrawBorder( gc, rect, UiGetColor( uiFocusFrameColor, 0, 0, 0 ) ); //CCC
		}
	}

	bool SButton::EventMouse( cevent_mouse* pEvent )
	{
		switch ( pEvent->Type() )
		{
			case EV_MOUSE_PRESS:
				break;

			case EV_MOUSE_RELEASE:
				if ( !IsEnabled() )
				{
					break;
				}

				if ( group > 0 )
				{
					if ( !IsSet() ) { Change( true ); }
				}
				else
				{
					Change( !IsSet() );
				}

				break;
		};

		return true;
	}

	bool SButton::EventKey( cevent_key* pEvent )
	{
		if (IsEnabled() &&
			pEvent->Type() == EV_KEYUP &&
			(pEvent->Key() == VK_SPACE || text.isHotkeyMatching(UnicodeUC(pEvent->Char())))
  			)
		{
			if ( group > 0 )
			{
				if ( !IsSet() ) { Change( true ); }
			}
			else
			{
				Change( !IsSet() );
			}

			return true;
		}

		return false;
	}

	Win* SButton::IsHisHotKey(cevent_key* pEvent)
	{
		return text.isHotkeyMatching(UnicodeUC(pEvent->Char())) ? this: 0;
	}

	bool SButton::EventFocus( bool recv )
	{
		Invalidate();
		return true;
	}


	SButton::~SButton() {}

}; // namespace wal
