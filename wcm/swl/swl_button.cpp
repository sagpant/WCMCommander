/*
   Copyright (c) by Valery Goryachev (Wal)
*/


#include "swl.h"
// XXX: refactor to move the header to .
#include "../unicode_lc.h" 

namespace wal
{

#define LEFTSPACE 5
#define RIGHTSPACE 5
#define ICONX_RIGHTSPACE 1

	int uiClassButton = GetUiID( "Button" );

	int Button::UiGetClassId()
	{
		return uiClassButton;
	}

	void Button::OnChangeStyles()
	{
		GC gc( this );
		gc.Set( GetFont() );
		cpoint p = text.GetTextExtents(gc);

		if ( icon.ptr() )
		{
			if ( p.y < 12 )
			{
				p.y = 12;
			}

			p.x += ICONX_RIGHTSPACE + icon->Width();

			if ( icon->Width() > p.y + 4 )
			{
				p.y = icon->Width() - 4;
			}
		}

		p.x += LEFTSPACE + RIGHTSPACE;
		p.x += 8;
		p.y += 8;
		SetLSize( LSize( p ) );
	}

	static unicode_t spaceUnicodeStr[] = {' ', 0};

	Button::Button( int nId, Win* parent, const unicode_t* txt, int id, crect* rect, int iconX, int iconY )
		:  Win( Win::WT_CHILD, Win::WH_TABFOCUS | WH_CLICKFOCUS, parent, rect, nId ),
		   pressed( false ),
		   icon( new cicon( id, iconX, iconY ) ),
		   commandId( id ),
		   text(txt && txt[0] ? txt : spaceUnicodeStr)
	{
		if ( !icon->Valid() )
		{
			icon.clear();
		}

		if ( !rect )
		{
			OnChangeStyles();
		}
	};

	void Button::Set( const unicode_t* txt, int id, int iconX, int iconY )
	{
		text.SetText( txt && txt[0] ? txt : spaceUnicodeStr );

		commandId = id;
		icon = new cicon( id, iconX, iconY );

		if ( !icon->Valid() )
		{
			icon.clear();
		}
	}

	bool Button::EventMouse( cevent_mouse* pEvent )
	{
		switch ( pEvent->Type() )
		{
			case EV_MOUSE_MOVE:
				if ( IsCaptured() )
				{
					crect r = ClientRect();
					cpoint p = pEvent->Point();

					if ( pressed )
					{
						if ( p.x < 0 || p.y < 0 || p.x >= r.right || p.y >= r.bottom )
						{
							pressed = false;
							Invalidate();
						}
					}
					else
					{
						if ( p.x >= 0 && p.y >= 0 && p.x < r.right && p.y < r.bottom )
						{
							pressed = true;
							Invalidate();
						}
					}
				}

				break;

			case EV_MOUSE_PRESS:
			case EV_MOUSE_DOUBLE:
				if ( pEvent->Button() != MB_L ) { break; }

				SetCapture();
				pressed = true;
				Invalidate();
				break;

			case EV_MOUSE_RELEASE:
				if ( pEvent->Button() != MB_L ) { break; }

				ReleaseCapture();

				if ( pressed ) { SendCommand(); }

				pressed = false;
				Invalidate();
				break;
		};

		return true;
	}

	bool Button::EventFocus( bool recv )
	{
		bool ret = Win::EventFocus( recv );

		if ( !recv && pressed )
		{
			pressed = false;
		}

		Invalidate();
		return ret;
	}

	bool Button::EventKey( cevent_key* pEvent )
	{
		if ((pEvent->Key() == VK_RETURN || pEvent->Key() == VK_NUMPAD_RETURN) || text.isHotkeyMatching(UnicodeUC(pEvent->Char())))
		{
			if (pEvent->Type() == EV_KEYDOWN)
			{
				pressed = true;
			}
			else if (pressed && pEvent->Type() == EV_KEYUP)
			{
				pressed = false;
				SendCommand();
			}
			else
			{
				return false;
			}
		}
		Invalidate();
		return true;
	}

	bool Button::IsMyHotKey(cevent_key* pEvent)
	{
		return text.isHotkeyMatching(UnicodeUC(pEvent->Char()));
	}

	void Button::Paint( GC& gc, const crect& paintRect )
	{
		unsigned colorBg = UiGetColor( uiBackground, 0, 0, 0x808080 ); //GetColor(0);
		crect cr = this->ClientRect();
		crect rect = cr;
		DrawBorder( gc, rect, ColorTone( colorBg, +20 ) );
		rect.Dec();
		DrawBorder( gc, rect, ColorTone( colorBg, -200 ) );
		rect.Dec();

		gc.SetFillColor( colorBg );
		gc.FillRect( rect );

		if ( pressed )
		{
#if USE_3D_BUTTONS
			Draw3DButtonW2( gc, rect, colorBg, false );
#endif
			rect.Dec();
			rect.Dec();
		}
		else
		{
#if USE_3D_BUTTONS
			Draw3DButtonW2( gc, rect, colorBg, true );
#endif
			rect.Dec();

			if ( InFocus() )
			{
				DrawBorder( gc, rect, /*GetColor(IC_FOCUS_MARK)*/ UiGetColor( uiFocusFrameColor, 0, 0, 0 ) );
			}

#if USE_3D_BUTTONS
			rect.Dec();
#endif
		}

		gc.SetTextColor( /*GetColor(IsEnabled() ? IC_TEXT : IC_GRAY_TEXT)*/ UiGetColor( uiColor, 0, 0, 0 ) );
		gc.Set( GetFont() );
		cpoint tsize = text.GetTextExtents(gc);

		/*
		int l = tsize.x + (icon.ptr() ? icon->Width() + ICONX_RIGHTSPACE : 0);

		int w = rect.Width() - LEFTSPACE - RIGHTSPACE;

		if (icon.ptr()) w-=ICONX_RIGHTSPACE;

		//int x = rect.left + LEFTSPACE + (w > l ? (w - l)/2 : 0) +(pressed?2:0);
		int x = rect.left + LEFTSPACE + (w-l)/2 +(pressed?2:0);
		*/

		int l = tsize.x + ( icon.ptr() ? icon->Width() + ICONX_RIGHTSPACE : 0 );
		int w = rect.Width();
		int x = rect.left + ( w > l ? ( w - l ) / 2 : 0 ) + ( pressed ? 2 : 0 );


		if ( icon.ptr() )
		{
			gc.DrawIcon( x, rect.top + ( rect.Height() - icon->Height() ) / 2 + ( pressed ? 2 : 0 ), icon.ptr() );
			x += icon->Width() + ICONX_RIGHTSPACE;
		}

		gc.SetClipRgn( &rect );
		text.DrawItem(gc, x, rect.top + (rect.Height() - tsize.y) / 2 + (pressed ? 2 : 0), UiGetColor(uiColor, 0, 0, 0), UiGetColor(uiHotkeyColor, 0, 0, 0));
	}

	Button::~Button() {}

}; //namespace wal
