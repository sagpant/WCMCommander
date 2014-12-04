/*
   Copyright (c) by Valery Goryachev (Wal)
*/


#include "swl.h"

namespace wal
{


#define SB_WIDTH 16

	int uiClassScrollBar = GetUiID( "ScrollBar" );
	int ScrollBar::UiGetClassId() {   return uiClassScrollBar; }

	void Draw3DButtonW1( GC& gc, crect r, unsigned bg, bool up )
	{
		static unsigned hp1, lp1;
		static unsigned lastBg = 0;
		static bool initialized = false;

		if ( !initialized || lastBg != bg )
		{
			hp1 = ColorTone( bg, 200 );
			lp1 = ColorTone( bg, -150 );
			lastBg = bg;
			initialized = true;
		}

		unsigned php1, plp1;

		if ( up )
		{
			php1 = hp1;
			plp1 = lp1;
		}
		else
		{
			php1 = lp1;
			plp1 = hp1;
		}

		gc.SetLine( plp1 );
		gc.MoveTo( r.right - 1, r.top );
		gc.LineTo( r.right - 1, r.bottom - 1 );
		gc.LineTo( r.left, r.bottom - 1 );
		gc.SetLine( php1 );
		gc.LineTo( r.left, r.top );
		gc.LineTo( r.right - 1, r.top );
	}

	/*
	types:
	   1 -left 2 - right 3 - horisontal slider
	   4 - up 5 - down   6 - vert. slider
	*/
	void SBCDrawButton( GC& gc, crect rect, int type, unsigned bg, bool pressed )
	{
		static unsigned short up[] = {6, 0x10, 0x38, 0x7c, 0xee, 0x1c7, 0x82};
		static unsigned short down[] = {6, 0x82, 0x1c7, 0xee, 0x7c, 0x38, 0x10,};
		static unsigned short left[] = {9, 0x10, 0x38, 0x1c, 0x0e, 0x07, 0x0e, 0x1c, 0x38, 0x10};
		static unsigned short right[] = {9, 0x02, 0x07, 0x0E, 0x1c, 0x38, 0x1c, 0x0e, 0x07, 0x02};
		DrawBorder( gc, rect, ColorTone( bg, -100 ) );
		rect.Dec();
		Draw3DButtonW1( gc, rect, bg, !pressed );
		//rect.Dec();
		rect.Dec();
		gc.SetFillColor( bg );
		gc.FillRect( rect );
		int xPlus = 0;
		int yPlus = 0;

		if ( pressed )
		{
			// xPlus = 1;
			yPlus = 1;
		}

		unsigned color = ColorTone( bg, -200 );

		switch ( type )
		{
			case 1:
				DrawPixelList( gc, left, rect.left + ( rect.Width() - 6 ) / 2 + xPlus, rect.top + ( rect.Height() - 9 ) / 2 + yPlus, color );
				break;

			case 2:
				DrawPixelList( gc, right, rect.left + ( rect.Width() - 6 ) / 2 + xPlus, rect.top + ( rect.Height() - 9 ) / 2 + yPlus, color );
				break;

			case 4:
				DrawPixelList( gc, up, rect.left + ( rect.Width() - 9 ) / 2 + xPlus, rect.top + ( rect.Height() - 6 ) / 2 + yPlus, color );
				break;

			case 5:
				DrawPixelList( gc, down, rect.left + ( rect.Width() - 9 ) / 2 + xPlus, rect.top + ( rect.Height() - 6 ) / 2 + yPlus, color );
				break;
		};
	}

	ScrollBar::ScrollBar( int nId, Win* parent, bool _vertical, bool _autoHide, crect* rect )
		:  Win( Win::WT_CHILD, 0, parent, rect, nId ),
		   vertical( _vertical ),
		   si( 100, 10, 0 ),
		   len( 0 ),
		   bsize( 0 ),
		   bpos( 0 ),
		   trace( false ),
		   traceBPoint( 0 ),
		   autoHide( _autoHide ),
		   b1Pressed( false ),
		   b2Pressed( false ),
		   managedWin( 0 )
	{
		LSize ls;
		int l = SB_WIDTH * 2;// + 2;// + 5;
		int h = SB_WIDTH;// + 2;

		if ( vertical )
		{
			ls.Set( cpoint( h, l ) );
			ls.y.maximal = 16000;
		}
		else
		{
			ls.Set( cpoint( l, h ) );
			ls.x.maximal = 16000;
		}

		SetLSize( ls );
		SetScrollInfo( nullptr );
//	crect r = ClientRect();
//	EventSize(&cevent_size(cpoint(r.Width(),r.Height())));
	}

	void ScrollBar::Paint( GC& gc, const crect& paintRect )
	{
		crect cr = ClientRect();
		unsigned bgColor = UiGetColor( uiBackground, 0, 0, 0xD8E9EC );/*GetColor(IC_SCROLL_BG)*/;
		unsigned btnColor = UiGetColor( uiButtonColor, 0, 0, 0xD8E9EC ); //GetColor(IC_SCROLL_BUTTON);
		gc.SetFillColor( bgColor );
		gc.FillRect( cr );
		DrawBorder( gc, cr, UiGetColor( uiColor, 0, 0, 0xD8E9EC )/*GetColor(IC_SCROLL_BORDER)*/ );

		if ( !b1Rect.IsEmpty() ) { SBCDrawButton( gc, b1Rect, vertical ? 4 : 1, btnColor, b1Pressed ); }

		if ( !b2Rect.IsEmpty() ) { SBCDrawButton( gc, b2Rect, vertical ? 5 : 2, btnColor, b2Pressed ); }

		if ( !b3Rect.IsEmpty() ) { SBCDrawButton( gc, b3Rect, vertical ? 6 : 3, btnColor, false ); }
	}

	void ScrollBar::SetScrollInfo( ScrollInfo* s )
	{
		ScrollInfo ns;

		if ( s ) { ns = *s; }
		else
		{
			ns.m_Size = ns.m_PageSize = ns.m_Pos = 0;
		}

		if ( ns.m_Size < 0 ) { ns.m_Size = 0; }

		if ( ns.m_PageSize < 0 ) { ns.m_PageSize = 0; }

		if ( ns.m_Pos < 0 ) { ns.m_Pos = 0; }

		if ( si == ns ) { return; }

		si = ns;

		Recalc();

		if ( si.m_AlwaysHidden )
		{
			Hide();
			if (Parent()) { Parent()->RecalcLayouts(); }
		}
		else if ( autoHide )
		{
			bool v = IsVisible();

			if ( si.m_Size <= si.m_PageSize && si.m_Pos <= 0 )
			{
				if ( v )
				{
					Hide();

					if ( Parent() ) { Parent()->RecalcLayouts(); }
				}
			}
			else
			{
				if ( !v )
				{
					Show();

					if ( Parent() ) { Parent()->RecalcLayouts(); }
				}
			}
		}
		else
		{
			Show();
			if (Parent()) { Parent()->RecalcLayouts(); }
		}

		Invalidate();
	}

	void ScrollBar::Recalc( cpoint* newSize )
	{
		cpoint cs;

		if ( newSize ) { cs = *newSize; }
		else
		{
			crect r = ClientRect();
			cs.Set( r.Width(), r.Height() );
		}

		int s = vertical ?   cs.y :   cs.x;
		b1Rect.Set( 0, 0, SB_WIDTH, SB_WIDTH );

		if ( vertical )
		{
			b2Rect.Set( 0, s - SB_WIDTH, SB_WIDTH, s );
		}
		else
		{
			b2Rect.Set( s - SB_WIDTH , 0 , s, SB_WIDTH );
		}

		len = s - SB_WIDTH * 2; // - 2;

		b3Rect.Zero();

		bpos = 0;

		if ( len <= 0 ) { return; }

		if ( si.m_Size <= 0 ) { return; }

		if ( si.m_PageSize <= 0 || si.m_PageSize >= si.m_Size ) { return; }

		bsize = int( ( int64_t( len ) * si.m_PageSize ) / si.m_Size );

		if ( bsize < 5 ) { bsize = 5; }

		if ( len <= bsize ) { return; }

		int space = ( len - bsize );

		bpos = int( ( int64_t( space ) * si.m_Pos ) / ( si.m_Size - si.m_PageSize ) );

		if ( bpos > space ) { bpos = space; }

		int n = SB_WIDTH + bpos;

		if ( vertical )
		{
			b3Rect.Set( 0, n, SB_WIDTH, n + bsize );
		}
		else
		{
			b3Rect.Set( n , 0 , n + bsize, SB_WIDTH );
		}
	}

	void ScrollBar::EventSize( cevent_size* pEvent )
	{
		Recalc( &( pEvent->Size() ) );
	}

	bool ScrollBar::Command( int id, int subId, Win* win, void* data )
	{
		if ( id != CMD_SCROLL_INFO || win != managedWin ) { return false; }

		if ( ( vertical && subId != SCMD_SCROLL_VCHANGE ) || ( !vertical && subId != SCMD_SCROLL_HCHANGE ) ) { return false; }

		SetScrollInfo( ( ScrollInfo* )data );
		return true;
	}

	void ScrollBar::SendManagedCmd( int subId, void* data )
	{
		if ( managedWin )
		{
			managedWin->Command( CMD_SCROLL_INFO, subId, this, data );
		}
		else if ( Parent() )
		{
			Parent()->Command( CMD_SCROLL_INFO, subId, this, data );
		}
	}

	void ScrollBar::EventTimer( int tid )
	{
		SendManagedCmd( tid, 0 );
	}

	bool ScrollBar::EventMouse( cevent_mouse* pEvent )
	{
		switch ( pEvent->Type() )
		{

			case EV_MOUSE_MOVE:
				if ( trace )
				{
					int p = vertical ? ( pEvent->Point().y - b1Rect.bottom )
					        : ( pEvent->Point().x - b1Rect.right );
					p -= traceBPoint;
					int n = len - bsize;

					if ( n <= 0 ) { break; }

					//if (p>=n) p = n-1;
					//if (p<0) p = 0;

					int n1 = int( si.m_Size - si.m_PageSize );
					int x = ( int64_t( p ) * n1 ) / n;

					if ( x >= n1 ) { x = n1; }

					if ( x < 0 ) { x = 0; }

					SendManagedCmd( SCMD_SCROLL_TRACK, &x ); //(void*)x);
				}

				break;

			case EV_MOUSE_DOUBLE:
			case EV_MOUSE_PRESS:
				if ( pEvent->Button() != MB_L )
				{
					break;
				}

				{
					int subId = 0;

					if ( b1Rect.In( pEvent->Point() ) )
					{
						b1Pressed = true;
						subId = ( vertical ) ? SCMD_SCROLL_LINE_UP : SCMD_SCROLL_LINE_LEFT;
					}
					else if ( b2Rect.In( pEvent->Point() ) )
					{
						b2Pressed = true;
						subId = ( vertical ) ? SCMD_SCROLL_LINE_DOWN : SCMD_SCROLL_LINE_RIGHT;
					}
					else if ( b3Rect.In( pEvent->Point() ) )
					{
						traceBPoint = ( vertical ) ? ( pEvent->Point().y - b3Rect.top ) : ( pEvent->Point().x - b3Rect.left );
						trace = true;
						SetCapture(&captureSD);
						break;
					}
					else if ( !b3Rect.IsEmpty() )
					{
						if ( vertical )
						{
							if ( pEvent->Point().y < b3Rect.top )
							{
								subId = SCMD_SCROLL_PAGE_UP;
							}
							else
							{
								subId = SCMD_SCROLL_PAGE_DOWN;
							}
						}
						else
						{
							if ( pEvent->Point().x < b3Rect.left )
							{
								subId = SCMD_SCROLL_PAGE_LEFT;
							}
							else
							{
								subId = SCMD_SCROLL_PAGE_RIGHT;
							}
						}
					}

					if ( subId != 0 )
					{
						SetCapture(&captureSD);
						SendManagedCmd( subId, 0 );
						SetTimer( subId, 100 );
					}
				}

				this->Invalidate();
				break;

			case EV_MOUSE_RELEASE:
				if ( pEvent->Button() != MB_L )
				{
					break;
				}

				if ( IsCaptured() )
				{
					b1Pressed = false ;
					b2Pressed = false ;
					ReleaseCapture(&captureSD);
					DelAllTimers();
					Invalidate();
					trace = false;
				}

				break;

			default:
				;
		}

		return true;
	}

	ScrollBar::~ScrollBar() {}


}; //namespace wal
