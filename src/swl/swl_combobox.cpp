/*
   Copyright (c) by Valery Goryachev (Wal)
*/


#include "swl.h"
#include "../globals.h"

namespace wal
{


#define CB_BUTTONWIDTH 16

	int uiClassComboBox = GetUiID( "ComboBox" );

	int ComboBox::UiGetClassId()
	{
		return uiClassComboBox;
	}


	void ComboBox::MoveCurrent( int n )
	{
		int saved = _current;

		if ( n < 0 )
		{
			if ( _flags & READONLY )
			{
				_edit.Clear();
			}

			_current = -1;

			if ( _box.ptr() )
			{
				_box->MoveCurrent( -1 );
			}
		}
		else if ( n < _list.count() )
		{
			_current = n;
			_edit.SetText( _list[n].text.data() );

			if ( _box.ptr() )
			{
				_box->MoveCurrent( n );
			}
		}

		if ( saved != _current )
		{
			Command( CMD_ITEM_CHANGED, _current, this, 0 );
		}
	}

	void ComboBox::OnChangeStyles()
	{
		RecalcLayouts();
		SetLSize( _lo.GetLSize() );
//	Win::OnChangeStyles();
	}


	ComboBox::ComboBox( int nId, Win* parent, int cols, int rows, unsigned flags,  crect* rect )
		:  Win( Win::WT_CHILD, WH_CLICKFOCUS | WH_TABFOCUS, parent, rect, nId ),
		   _flags( flags ),
		   _lo( 3, 4 ),
		   _edit( 0, this, 0, 0, cols, false, EditLine::USEPARENTFOCUS | ( ( flags& READONLY ) ? EditLine::READONLY : 0 ) ),
		   _cols( cols > 0 ? cols : 10 ),
		   _rows( rows > 0 ? rows : 0 )
	{
		_lo.AddWin( &_edit, 1, 1 );
		_edit.SetClickFocusFlag( false );
		_edit.SetTabFocusFlag( false );
		_edit.Enable();
		_edit.Show( SHOW_INACTIVE );
		_lo.AddRect( &_buttonRect, 1, 2 );
		_lo.ColSet( 2, CB_BUTTONWIDTH );
		int fw = ( _flags & FRAME3D ) != 0 ? 3 : ( ( _flags & NOFOCUSFRAME ) ? 0 : 1 );
		_lo.ColSet( 0, fw );
		_lo.ColSet( 3, fw );
		_lo.LineSet( 0, fw );
		_lo.LineSet( 2, fw );
		SetLayout( &_lo );
	}

	void ComboBox::Clear()
	{
		if ( _box.ptr() ) { _box->Clear(); }

		//CloseBox();
		_list.clear();

		if ( _current >= 0 )
		{
			_current = -1;

			if ( IsVisible() ) { Invalidate(); }
		}
	}

	void ComboBox::Append( const unicode_t* text, void* data )
	{
		//CloseBox();
		Node node;
		node.text = new_unicode_str( text );
		node.data = data;
		_list.append( node );
	}

	void ComboBox::Append( const char* text, void* data )
	{
		Append( utf8_to_unicode( text ).data(), data );
	}

	const unicode_t* ComboBox::ItemText( int n )
	{
		static unicode_t empty = 0;
		return ( n >= 0 && n < _list.count() ) ? _list[n].text.data() : &empty;
	}

	std::vector<unicode_t> ComboBox::GetText() const
	{
		//static unicode_t empty = 0;
		return _edit.GetText();
	}

	void ComboBox::SetText( const unicode_t* txt, bool mark )
	{
		if ( this->_flags & READONLY ) { return; }

		_edit.SetText( txt, mark );
		MoveCurrent( -1 );
	}

	void ComboBox::InsertText( unicode_t t )
	{
		if ( this->_flags & READONLY ) { return; }

		_edit.Insert( t );
	}

	void ComboBox::InsertText( const unicode_t* txt )
	{
		if ( this->_flags & READONLY ) { return; }

		_edit.Insert( txt );
	}


	void* ComboBox::ItemData( int n )
	{
		return ( n >= 0 && n < _list.count() ) ? _list[n].data : 0;
	}

	bool ComboBox::Command( int id, int subId, Win* win, void* d )
	{
		if ( win == _box.ptr() && _box.ptr() )
		{
			int n = _box->GetCurrent();

			if ( n >= 0 && n < _list.count() && n != _current )
			{
				_current = n;
				_edit.SetText( ItemText( n ) );
				Command( CMD_ITEM_CHANGED, _current, this, 0 );
			}

			if ( id == CMD_ITEM_CLICK ) //иначе зациклится
			{
				CloseBox();
			}

			return true;
		}

		if ( win == &_edit && _current != -1 )
		{
			_current = -1;

			if ( _box ) { _box->SetNoCurrent(); }

			Command( CMD_ITEM_CHANGED, _current, this, 0 );
			return true;
		};

		return Win::Command( id, subId, win, d );
	}

	bool ComboBox::EventFocus( bool recv )
	{
		_edit.EventFocus( recv );

		if ( !recv && _box.ptr() ) { CloseBox(); }

		Invalidate();
		return true;
	}

	bool ComboBox::EventKey( cevent_key* pEvent )
	{
		if ( pEvent->Type() == EV_KEYDOWN )
		{
			unsigned mod = pEvent->Mod();
			unsigned fullKey = ( pEvent->Key() & 0xFFFF ) + ( mod << 16 );

#define FC(key, mods) (((key)&0xFFFF) + ((mods)<<16))

			switch ( fullKey )
			{
				case VK_ESCAPE:
					if ( _box.ptr() )
					{
						CloseBox();
						return true;
					}

					break;

				case VK_RETURN:
					if ( _box.ptr() )
					{
						CloseBox();
						return _current >= 0;
					}

					break;

				case FC( VK_UP, KM_SHIFT ):
				case FC( VK_UP, KM_CTRL ):
				case FC( VK_DOWN, KM_SHIFT ):
				case FC( VK_DOWN, KM_CTRL ):
					if ( _box.ptr() )
					{
						CloseBox();
						return true;
					}
					else
					{
						OpenBox();
						return true;
					}

					return true;


				case VK_UP:
					if ( _box.ptr() )
					{
						return _box->EventKey( pEvent );
					}
					else
					{
						if ( _current > 0 )
						{
							MoveCurrent( _current - 1 );
							return true;
						}
					}

					break;

				case VK_DOWN:
					if ( _box.ptr() )
					{
						return _box->EventKey( pEvent );
					}
					else
					{
						if ( _current + 1 < _list.count() )
						{
							MoveCurrent( _current + 1 );
							return true;
						}
					}

					break;

				case VK_NEXT:
				case VK_PRIOR:
					if ( _box.ptr() )
					{
						return _box->EventKey( pEvent );
					}

					break;
			}

			return _edit.EventKey( pEvent );
		}

		return false;
	}

	bool ComboBox::OnOpenBox()
	{
		return true;
	}

	void ComboBox::OnCloseBox()
	{
	}

	void ComboBox::RefreshBox()
	{
		if ( !_box.ptr() ) { return; }

		_box->Clear();

		for ( int i = 0; i < _list.count(); i++ )
		{
			_box->Append( _list[i].text.data() );
		}

		_box->DataRefresh();
	}

	void ComboBox::OpenBox()
	{
		if ( _box.ptr() ) { return; }

		if ( !OnOpenBox() ) { return; }

		_box = new TextList( Win::WT_POPUP, 0, 0, this, VListWin::SINGLE_SELECT,  VListWin::SINGLE_BORDER, 0 );

		for ( int i = 0; i < _list.count(); i++ )
		{
			_box->Append( _list[i].text.data() );
		}

		if ( _current >= 0 ) { _box->MoveCurrent( _current ); }

		_box->SetHeightRange( LSRange( _rows, _rows, _rows ) );

		int width = ClientRect().Width();

		if ( width < 20 ) { width = 20; }

		LSize ls;
		_box->GetLSize( &ls );

		crect rect = this->ScreenRect();

		if ( _flags & MODE_UP )
		{
			rect.bottom = rect.top;
			rect.top = rect.bottom - ls.y.minimal;
		}
		else
		{
			rect.top = rect.bottom;
			rect.right = rect.left + width;
			rect.bottom = rect.top + ls.y.minimal;
		}

		_box->Move( rect );
		_box->Show( Win::SHOW_INACTIVE );
		_box->Enable();
		SetCapture( &captureSD );
	}

	void ComboBox::CloseBox()
	{
		if ( IsCaptured() ) { ReleaseCapture( &captureSD ); }

		if ( _box.ptr() )
		{
			OnCloseBox();
			_box = 0;
		}
	}

	bool ComboBox::EventMouse( cevent_mouse* pEvent )
	{
		if ( pEvent->Type() == EV_MOUSE_PRESS && InFocus() && _buttonRect.In( pEvent->Point() ) )
		{
			if ( _box.ptr() )
			{
				CloseBox();
			}
			else
			{
				OpenBox();
			}

			return true;
		}

		if ( IsCaptured() )
		{
			crect rect = ScreenRect();
			cpoint point = pEvent->Point();

			point.x += rect.left;
			point.y += rect.top;


			rect = _edit.ScreenRect();

			if ( rect.In( point ) )
			{
				cevent_mouse ev( pEvent->Type(),
				                 cpoint( point.x - rect.left, point.y - rect.top ),
				                 pEvent->Button(),
				                 pEvent->ButtonFlag(),
				                 pEvent->Mod() );
				return _edit.EventMouse( &ev );
			}
			else if ( _box.ptr() )
			{
				rect = _box->ScreenRect();

				if ( rect.In( point ) )
				{
					cevent_mouse ev( pEvent->Type(),
					                 cpoint( point.x - rect.left,
					                         point.y - rect.top ), pEvent->Button(),
					                 pEvent->ButtonFlag(),
					                 pEvent->Mod() );
					return _box->EventMouse( &ev );
				}
				else if ( pEvent->Type() == EV_MOUSE_PRESS )
				{
					CloseBox();
				}
			}
		}

		return true;
	}


	extern void SBCDrawButton( GC& gc, crect rect, int type, unsigned bg, bool pressed );


	void ComboBox::Paint( GC& gc, const crect& paintRect )
	{
		crect cr = ClientRect();
		unsigned bgColor = UiGetColor( uiBackground, 0, 0, 0xD8E9EC );
//	unsigned btnColor = UiGetColor(uiButtonColor, 0, 0, 0xD8E9EC);

		gc.SetFillColor( bgColor );

		unsigned frameColor = UiGetColor( uiFrameColor, 0, 0, 0xFFFFFF );

		if ( ( _flags & FRAME3D ) != 0 )
		{
			if ( g_WcmConfig.styleShow3DUI )
			{
				Draw3DButtonW2( gc, cr, frameColor, false );
				cr.Dec();
				cr.Dec();
			}
			else
			{
				DrawBorder( gc, cr, frameColor );
				cr.Dec();
				DrawBorder( gc, cr, frameColor );
				cr.Dec();
			}

			/*
			      Draw3DButtonW2(gc, cr, bgColor, false);
			      cr.Dec();
			      cr.Dec();
			*/
		}

		DrawBorder( gc, cr, InFocus() && ( _flags & NOFOCUSFRAME ) == 0 ? UiGetColor( uiFocusFrameColor, 0, 0, 0 ) : ColorTone( UiGetColor( uiFrameColor, 0, 0, 0xFFFFFF ), -200 ) );

		SBCDrawButton( gc, _buttonRect, ( _flags & MODE_UP ) ? 4 : 5, bgColor, false );
	}


	ComboBox::~ComboBox() {}


} //namespace wal
