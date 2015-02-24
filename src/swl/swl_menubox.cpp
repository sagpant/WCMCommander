/*
   Copyright (c) by Valery Goryachev (Wal)
*/


#include "swl.h"
// XXX: refactor to move the header to .
#include "../unicode_lc.h"
namespace wal
{


	int uiClassMenuBar = GetUiID( "MenuBar" );
	int MenuBar::UiGetClassId() {  return uiClassMenuBar; }

	void MenuBar::OnChangeStyles()
	{
		LSize ls;
		ls.x.maximal = 10000;
		ls.x.minimal = 0;
		ls.x.ideal = 100;

		GC gc( this );
		gc.Set( GetFont() );

		cpoint p = gc.GetTextExtents( ABCString );
		ls.y.ideal = ls.y.minimal = ls.y.maximal = p.y + 6;//4;// + 6 ;

		SetLSize( ls );
	}

	MenuBar::MenuBar( int nId, Win* parent, crect* rect )
		:  Win( Win::WT_CHILD, 0 /*Win::WH_CLICKFOCUS*/, parent, rect, nId ),
		   select( -1 ),
		   lastMouseSelect( -1 )
	{
		OnChangeStyles();
	}


	void MenuBar::OpenSub()
	{
		ReleaseCapture();
		sub = 0;

		if ( select >= 0 && select < list.count() )
		{
			crect rect = ScreenRect();
			crect itemRect = ItemRect( select );
			int x = rect.left + itemRect.left;
			int y = rect.top + itemRect.bottom;
			sub = new PopupMenu( 0, this, list[select].data, x, y, this ); //Parent() ? Parent() : 0);
			sub->Show( Win::SHOW_INACTIVE );
			sub->Enable( true );
			SetCapture();
		}
	}

	int MenuBar::GetSelect() const
	{
		return select;
	}

	void MenuBar::SetSelect( int n )
	{
		if ( select == n ) { return; }

//		int t = select;
		select = n;

		if ( sub.ptr() )
		{
			OpenSub();
		}

		if ( IsVisible() ) { /*GC gc(this); DrawItem(gc,t); DrawItem(gc,select);*/ Invalidate(); }
	}

	int MenuBar::GetPointItem( cpoint p )
	{
		for ( int i = 0; i < list.count(); i++ )
			if ( ItemRect( i ).In( p ) )
			{
				return i;
			}

		return -1;
	}

	bool MenuBar::EventMouse( cevent_mouse* pEvent )
	{
		int n;

		if ( pEvent->Type() == EV_MOUSE_PRESS )
		{
			n = GetPointItem( pEvent->Point() );

			if ( n >= 0 )
			{
				SetFocus();
				SetSelect( n );
				OpenSub();
				return true;
			}

			if ( sub.ptr() )
			{
				crect rect = ScreenRect();
				crect sr = sub->ScreenRect();
				pEvent->Point().x += rect.left - sr.left;
				pEvent->Point().y += rect.top - sr.top;

				if ( sub->EventMouse( pEvent ) )
				{
					SetSelect( -1 );
					return true;
				}

				SetSelect( -1 );

				if ( Parent() ) { Parent()->Command( CMD_MENU_INFO, SCMD_MENU_CANCEL, this, 0 ); }
			}

			return true;
		}
		else  if ( pEvent->Type() == EV_MOUSE_MOVE )
		{
			n = GetPointItem( pEvent->Point() );

			if ( n >= 0 )
			{
				if ( lastMouseSelect != n )
				{
					lastMouseSelect = n;
					SetSelect( n );
				}

				return true;
			}

			if ( !sub.ptr() && !InFocus() )
			{
				lastMouseSelect = -1;
				SetSelect( -1 );

				if ( Parent() ) { Parent()->Command( CMD_MENU_INFO, SCMD_MENU_CANCEL, this, 0 ); }

				return true;
			}

			if ( sub.ptr() )
			{
				crect rect = ScreenRect();
				crect sr = sub->ScreenRect();
				pEvent->Point().x += rect.left - sr.left;
				pEvent->Point().y += rect.top - sr.top;
				return sub->EventMouse( pEvent );
			}

			return true;
		}

		return false;
	}

	bool MenuBar::EventKey( cevent_key* pEvent )
	{
		if ( sub.ptr() && sub->EventKey( pEvent ) ) { return true; }

		if ( pEvent->Type() == EV_KEYDOWN )
		{
			switch ( pEvent->Key() )
			{
				case VK_LEFT:
					Left();
					return true;

				case VK_RIGHT:
					Right();
					return true;

				case VK_DOWN:
				case VK_RETURN:
				case VK_NUMPAD_RETURN:
					OpenSub();
					return true;

				case VK_ESCAPE:
					SetSelect( -1 );

					if ( Parent() )
					{
						Parent()->Command( CMD_MENU_INFO, SCMD_MENU_CANCEL, this, 0 );
					}

					return true;

				default:
				{
					// check if hotkey matches, and process
					// XXX: pEvent->Key() returns key (like Shift-F1, etc). isHotkeyMatching() expects unicode char, which is not the same

					unicode_t c = UnicodeUC( pEvent->Char() );

					for ( int i = 0; i < list.count(); i++ )
					{
						MenuBar::Node& node = list[i];

						if ( node.text.isHotkeyMatching( c ) )
						{
							SetSelect( i );
							OpenSub();
							return true;
						}
					}

					return false;
				}
			}
		}

		return false;
	}

	bool MenuBar::EventFocus( bool recv )
	{
		if ( recv )
		{
			if ( select < 0 || select >= list.count() ) { SetSelect( 0 ); }
		}
		else if ( !sub.ptr() )
		{
			SetSelect( -1 );
		}

		return true;
	}

	void MenuBar::EventEnterLeave( cevent* pEvent )
	{
		if ( pEvent->Type() == EV_LEAVE && !InFocus() && !sub.ptr() )
		{
			SetSelect( -1 );
		}

	}

	bool MenuBar::Command( int id, int subId, Win* win, void* d )
	{
		Win* P = Parent();

		if ( id == CMD_CHECK )
		{
			return P ? P->Command( id, subId, this, d ) : false;
		}

		SetSelect( -1 );

		if ( IsModal() )
		{
			EndModal( id );
		}

		if ( P ) { P->Command( CMD_MENU_INFO, SCMD_MENU_SELECT, this, nullptr ); }

		return P ? P->Command( id, subId, win, d ) : false;
	}


	void MenuBar::Add( MenuData* data, const unicode_t* text )
	{
		Node node( MenuTextInfo( text ), data );
		//node.text = new_unicode_str( text );
		//node.data = data;
		list.append( node );
		InvalidateRectList();

		if ( IsVisible() ) { Invalidate(); }
	}


	void MenuBar::DrawItem( GC& gc, int n )
	{
		if ( n < 0 || n >= list.count() ) { return; }

		UiCondList ucl;

		if ( n == select && InFocus() ) { ucl.Set( uiCurrentItem, true ); }

		int color_text = UiGetColor( uiColor, uiItem, &ucl, 0x0 );
		int color_hotkey = UiGetColor( uiHotkeyColor, uiItem, &ucl, 0x0 );
		int color_bg = UiGetColor( uiBackground, uiItem, &ucl, 0xFFFFFF );

		gc.Set( GetFont() );
		crect itemRect = ItemRect( n );

		gc.SetFillColor( color_bg );

		if ( n == select && InFocus() ) { gc.FillRect( itemRect ); }

		/*
		// https://github.com/corporateshark/WalCommander/issues/223
		if ( n == select )
		{
		   DrawBorder( gc, itemRect, UiGetColor( uiCurrentItemFrame, uiItem, &ucl, 0xFFFFFF ) );
		}
		*/
		gc.SetTextColor( color_text );

		MenuTextInfo& mt = list[n].text;
		cpoint tsize = mt.GetTextExtents( gc );
		int x = itemRect.left + ( itemRect.Width() - tsize.x ) / 2;
		int y = itemRect.top + ( itemRect.Height() - tsize.y ) / 2;

		mt.DrawItem( gc, x, y, color_text, color_hotkey );
	}


	MenuBar::~MenuBar() {}


	void MenuBar::EventSize( cevent_size* pEvent )
	{
		Win::EventSize( pEvent );
		rectList.clear();
	}

	crect MenuBar::ItemRect( int n )
	{
		if ( !rectList.count() )
		{
			crect wRect = ClientRect();
			GC gc( this );
			gc.Set( GetFont() );
			cpoint chSize = gc.GetTextExtents( ABCString );
			int spaceWidth = chSize.x / ABCStringLen;
			int x = 1;

			for ( int i = 0; i < list.count(); i++ )
			{
				cpoint textSize = list[i].text.GetTextExtents( gc );
				int x2 = x + textSize.x + spaceWidth * 2 + 2 ;
				crect itemRect( x, 1, x2 , wRect.bottom - 1 );
				x = x2;
				rectList.append( itemRect );
			}
		}

		return ( n < 0 || n >= list.count() ) ? crect( 0, 0, 0, 0 ) : rectList[n];
	}



	inline unsigned MidAB( int a, int b, int i, int n )
	{
		return n > 1 ? a + ( ( b - a ) * i ) / ( n - 1 ) : a;
	}

	void FillHorisontalRect( wal::GC& gc, crect rect, unsigned a, unsigned b )
	{
		unsigned   ar = a & 0xFF,    ag = ( a >> 8 ) & 0xFF, ab = ( a >> 16 ) & 0xFF,
		           br = b & 0xFF, bg = ( b >> 8 ) & 0xFF, bb = ( b >> 16 ) & 0xFF;

		int h = rect.Height();
		int x1 = rect.left, x2 = rect.right;

		if ( h <= 0 || x1 >= x2 ) { return; }

		for ( int i = 0; i < h; i++ )
		{
			unsigned color = ( MidAB( ar, br, i, h ) & 0xFF ) + ( ( MidAB( ag, bg, i, h ) & 0xFF ) << 8 ) + ( ( MidAB( ab, bb, i, h ) & 0xFF ) << 16 );
			gc.SetLine( color );
			gc.MoveTo( x1, rect.top + i );
			gc.LineTo( x2, rect.top + i );
		}

	}


	void MenuBar::Paint( GC& gc, const crect& paintRect )
	{
		crect rect = ClientRect();

		unsigned color  = UiGetColor( uiBackground, 0, 0, 0xFFFFFF );
		unsigned bColor = ColorTone( color, -50 ), aColor = ColorTone( color, +50 );
		FillHorisontalRect( gc, rect, aColor, bColor );

		for ( int i = 0; i < list.count(); i++ )
		{
			DrawItem( gc, i );
		}
	}

}; //namespace wal
