/*
   Copyright (c) by Valery Goryachev (Wal)
*/


#include "swl.h"
#include "../globals.h"
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
		cpoint p = m_Text.GetTextExtents( gc );

		if ( HasIcon() )
		{
			if ( p.y < 12 )
			{
				p.y = 12;
			}

			p.x += ICONX_RIGHTSPACE + m_Icon->Width();

			if ( m_Icon->Width() > p.y + 4 )
			{
				p.y = m_Icon->Width() - 4;
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
		   m_Pressed( false ),
		   m_Text( txt && txt[0] ? txt : spaceUnicodeStr ),
		   m_Icon( new cicon( id, iconX, iconY ) ),
		   m_CommandId( id ),
		   m_ShowIcon( true )
	{
		if ( !m_Icon->Valid() )
		{
			m_Icon.clear();
		}

		if ( !rect )
		{
			OnChangeStyles();
		}
	};

	void Button::Set( const unicode_t* txt, int id, int iconX, int iconY )
	{
		m_Text.SetText( txt && txt[0] ? txt : spaceUnicodeStr );

		m_CommandId = id;
		m_Icon = new cicon( id, iconX, iconY );

		if ( !m_Icon->Valid() )
		{
			m_Icon.clear();
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

					if ( m_Pressed )
					{
						if ( p.x < 0 || p.y < 0 || p.x >= r.right || p.y >= r.bottom )
						{
							m_Pressed = false;
							Invalidate();
						}
					}
					else
					{
						if ( p.x >= 0 && p.y >= 0 && p.x < r.right && p.y < r.bottom )
						{
							m_Pressed = true;
							Invalidate();
						}
					}
				}

				break;

			case EV_MOUSE_PRESS:
			case EV_MOUSE_DOUBLE:
				if ( pEvent->Button() != MB_L ) { break; }

				SetCapture();
				m_Pressed = true;
				Invalidate();
				break;

			case EV_MOUSE_RELEASE:
				if ( pEvent->Button() != MB_L ) { break; }

				ReleaseCapture();

				if ( m_Pressed ) { SendCommand(); }

				m_Pressed = false;
				Invalidate();
				break;
		};

		return true;
	}

	bool Button::EventFocus( bool recv )
	{
		bool ret = Win::EventFocus( recv );

		if ( !recv && m_Pressed )
		{
			m_Pressed = false;
		}

		Invalidate();
		return ret;
	}

	bool Button::EventKey( cevent_key* pEvent )
	{
		bool IsReturn = pEvent->Key() == VK_RETURN || pEvent->Key() == VK_NUMPAD_RETURN;
		bool IsSpace = pEvent->Key() == VK_SPACE;
		bool IsHotkey = m_Text.isHotkeyMatching( UnicodeUC( pEvent->Char() ) );

		if ( IsReturn || IsSpace || IsHotkey )
		{
			if ( pEvent->Type() == EV_KEYDOWN )
			{
				m_Pressed = true;
			}
			else if ( m_Pressed && pEvent->Type() == EV_KEYUP )
			{
				m_Pressed = false;
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

	Win*  Button::IsHisHotKey( cevent_key* pEvent )
	{
		return m_Text.isHotkeyMatching( UnicodeUC( pEvent->Char() ) ) ? this : nullptr;
	}

	void Button::Paint( GC& gc, const crect& paintRect )
	{
		unsigned colorBg = UiGetColor( uiBackground, uiClassButton, 0, 0x808080 ); //GetColor(0);
		crect cr = this->ClientRect();
		crect rect = cr;
		DrawBorder( gc, rect, ColorTone( colorBg, +20 ) );
		rect.Dec();
		DrawBorder( gc, rect, ColorTone( colorBg, -200 ) );
		rect.Dec();

		gc.SetFillColor( colorBg );
		gc.FillRect( rect );

		if ( m_Pressed )
		{
			if ( g_WcmConfig.styleShow3DUI )
			{
				Draw3DButtonW2( gc, rect, colorBg, false );
			}

			rect.Dec();
			rect.Dec();
		}
		else
		{
			if ( g_WcmConfig.styleShow3DUI )
			{
				Draw3DButtonW2( gc, rect, colorBg, true );
			}

			rect.Dec();

			if ( InFocus() )
			{
				DrawBorder( gc, rect, /*GetColor(IC_FOCUS_MARK)*/ UiGetColor( uiFocusFrameColor, 0, 0, 0 ) );
			}
		}

		//gc.SetTextColor( /*GetColor(IsEnabled() ? IC_TEXT : IC_GRAY_TEXT)*/ UiGetColor( uiColor, 0, 0, 0 ) );
		gc.Set( GetFont() );
		cpoint tsize = m_Text.GetTextExtents( gc );

		/*
		int l = tsize.x + (icon.ptr() ? icon->Width() + ICONX_RIGHTSPACE : 0);

		int w = rect.Width() - LEFTSPACE - RIGHTSPACE;

		if (icon.ptr()) w-=ICONX_RIGHTSPACE;

		//int x = rect.left + LEFTSPACE + (w > l ? (w - l)/2 : 0) +(pressed?2:0);
		int x = rect.left + LEFTSPACE + (w-l)/2 +(pressed?2:0);
		*/

		bool HasIcon = m_Icon.ptr() && m_ShowIcon;

		int l = tsize.x + ( HasIcon ? m_Icon->Width() + ICONX_RIGHTSPACE : 0 );
		int w = rect.Width();
		int x = rect.left + ( w > l ? ( w - l ) / 2 : 0 ) + ( m_Pressed ? 2 : 0 );


		if ( HasIcon )
		{
			gc.DrawIcon( x, rect.top + ( rect.Height() - m_Icon->Height() ) / 2 + ( m_Pressed ? 2 : 0 ), m_Icon.ptr() );
			x += m_Icon->Width() + ICONX_RIGHTSPACE;
		}

		gc.SetClipRgn( &rect );
		m_Text.DrawItem(
		   gc, x, rect.top + ( rect.Height() - tsize.y ) / 2 + ( m_Pressed ? 2 : 0 ),
		   UiGetColor( uiColor, uiClassButton, 0, 0 ),
		   UiGetColor( uiHotkeyColor, uiClassButton, 0, 0 )
		);
	}

	Button::~Button() {}

}; //namespace wal
