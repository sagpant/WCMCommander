#include "toolbar.h"


const int SPLITTER_WIDTH = 6;
const int XSPACE = 3;
const int YSPACE = 1;

ToolBar::ToolBar( Win* parent, const crect* rect, int iconSize )
	:  Win( Win::WT_CHILD, 0, parent, rect ),
	   _iconSize( iconSize ),
	   _pressed( 0 ),
	   _ticks( 0 ),
	   _curTip( 0 ),
	   _nextTip( 0 )
{
	OnChangeStyles();
}

int uiClassToolBar = GetUiID( "ToolBar" );

int ToolBar::UiGetClassId() { return uiClassToolBar; }

void ToolBar::RecalcItems()
{
	crect cr = ClientRect();
	int x = 2;
	int y = 2;

	wal::GC gc( this );
	gc.Set( GetFont() );

	for ( int i = 0; i < _list.count(); i++ )
	{
		Node* p = _list[i].ptr();

		if ( p )
		{
			int w = 0;
			int h = _iconSize;

			if ( p->icon.ptr() && p->icon->Valid() )
			{
				w += _iconSize;
			};

			if ( w )
			{
				w += 2 + XSPACE * 2;
				h += 2 + YSPACE * 2;
				p->rect.Set( x, y, x + w, y + h );
				x += w;
			}
			else { p->rect.Set( 0, 0, 0, 0 ); }

		}
		else
		{
			x += SPLITTER_WIDTH;
		}
	}
}

void ToolBar::Clear()
{
	_list.clear();
}


void ToolBar::AddCmd( int cmd, const char* tipText )
{
	if ( cmd != 0 )
	{
		clPtr<Node> p = new Node();
		p->cmd = cmd;

		if ( tipText && tipText[0] ) { p->tipText = utf8_to_unicode( tipText ); }

		p->icon = new cicon( cmd, _iconSize, _iconSize );
		_list.append( p );
		RecalcItems();
	}
}


void ToolBar::AddSplitter()
{
	if ( _list.count() > 0 && _list[_list.count() - 1].ptr() )
	{
		_list.append( clPtr<Node>( 0 ) );
	}
}

void ToolBar::OnChangeStyles()
{
	int h = _iconSize;
	h += 2 + 4 +  YSPACE * 2;

	LSize ls;
	ls.x.minimal = ls.x.ideal = 0;
	ls.x.maximal = 16000;
	ls.y.minimal = ls.y.maximal = ls.y.ideal = h;
	SetLSize( ls );
	RecalcItems();
}

ToolBar::Node* ToolBar::GetNodeByPos( int x, int y )
{
	for ( int i = 0; i < _list.count(); i++ )
	{
		Node* p = _list[i].ptr();

		if ( p && p->rect.In( cpoint( x, y ) ) )
		{
			return p;
		}
	}

	return 0;
}

void ToolBar::DrawNode( wal::GC& gc, Node* pNode, int state )
{
	if ( !pNode ) { return; }

	int W = pNode->rect.Width();
	int H = pNode->rect.Height();

	if ( W <= 0 || H <= 0 ) { return; }

	int x = pNode->rect.left + 1 + XSPACE;
	int y = pNode->rect.top + 1 + YSPACE;

	unsigned bgColor = UiGetColor( uiBackground, uiItem, 0, 0x808080 ); //GetColor(0);
	unsigned frameColor = ColorTone( bgColor, -150 );

	if ( state == DRAW_PRESSED )
	{
		bgColor = ColorTone( bgColor, -50 );
	}

	gc.SetFillColor( bgColor );

	{
		crect r = pNode->rect;
		r.Dec();
		gc.FillRect( r );
	}

	if ( pNode->icon.ptr() && pNode->icon->Valid() )
	{
		pNode->icon->DrawF( gc, x, y );
		x += _iconSize;
	}

	if ( state != DRAW_NORMAL )
	{
		DrawBorder( gc, pNode->rect, frameColor );
	}
}

void ToolBar::Paint( wal::GC& gc, const crect& paintRect )
{
	crect cr = ClientRect();
	crect rect = cr;
	unsigned colorBg = UiGetColor( uiBackground, 0, 0, 0x808080 ); //GetColor(0);

	unsigned splitColor1 = ColorTone( colorBg, -70 );
	unsigned splitColor2 = ColorTone( colorBg, 70 );

#if USE_3D_BUTTONS
	Draw3DButtonW2( gc, rect, colorBg, true );
	rect.Dec();
	rect.Dec();
#endif

	gc.SetFillColor( colorBg );
	gc.FillRect( rect );

	gc.Set( GetFont() );
	int x = 2;

	for ( int i = 0; i < _list.count(); i++ )
	{
		Node* p = _list[i].ptr();

		if ( p )
		{
			DrawNode( gc, p, _pressed == p ? DRAW_PRESSED : DRAW_NORMAL );
			x += p->rect.Width();
		}
		else
		{
			gc.SetFillColor( splitColor1 );
			int x1 = x + SPLITTER_WIDTH / 2;
			gc.FillRect( crect( x1, rect.top, x1 + 1, rect.bottom ) );
			gc.SetFillColor( splitColor2 );
			gc.FillRect( crect( x1 + 2, rect.top, x1 + 2, rect.bottom ) );
			x += SPLITTER_WIDTH;

		}
	}

	return;
}

void ToolBar::EventTimer( int tid )
{
	if ( _curTip != _nextTip )
	{
		_ticks++;

		if ( _ticks > 0 )
		{
			_curTip = _nextTip;

			if ( _nextTip && _nextTip->tipText.data() && _nextTip->tipText[0] )
			{
				ToolTipShow( this, _nextTip->rect.left + 3, _nextTip->rect.bottom + 5, _nextTip->tipText.data() );
			}
		}
	}
}

bool ToolBar::EventMouse( cevent_mouse* pEvent )
{
	{
		_ticks = 0;
		//_mPoint = pEvent->Point();
		_nextTip = GetNodeByPos( pEvent->Point().x, pEvent->Point().y );

		if ( _curTip && !_nextTip )
		{
			ToolTipHide();
			_curTip = _nextTip = 0;
		}
	}

	switch ( pEvent->Type() )
	{
		case EV_MOUSE_PRESS:
		case EV_MOUSE_DOUBLE:
		{
			ToolTipHide();
			_curTip = _nextTip = 0;
			DelAllTimers();

			if ( pEvent->Button() != MB_L ) { break; }

			Node* p = GetNodeByPos( pEvent->Point().x, pEvent->Point().y );

			if ( !p ) { break; }

			SetCapture();
			_pressed = p;
			Invalidate();
		}
		break;

		case EV_MOUSE_RELEASE:
		{
			ToolTipHide();
			_curTip = _nextTip = 0;
			DelAllTimers();


			if ( pEvent->Button() != MB_L ) { break; }

			if ( !_pressed ) { break; }

			ReleaseCapture();
			Node* p = GetNodeByPos( pEvent->Point().x, pEvent->Point().y );

			if ( _pressed == p )
			{
				Command( p->cmd, 0, this, 0 );
			}

			_pressed = 0;
			Invalidate();
		}
		break;

	};

	return true;
}

void ToolBar::EventEnterLeave( cevent* pEvent )
{
	if ( pEvent->Type() == EV_LEAVE )
	{
		DelAllTimers();
		ToolTipHide();
	}
	else
	{
//printf("TIMER\n");
		DelAllTimers();
		SetTimer( 0, 200 );
	}
}

ToolBar::~ToolBar()
{
}