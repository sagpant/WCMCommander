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

	void ComboBox::MoveCurrent( int n, bool notify )
	{
		int saved = _current;

		if ( n < 0 )
		{
			_current = -1;

			if ( _box.ptr() )
			{
				_box->MoveCurrent( -1 );
			}
		}
		else if ( n < _list.count() )
		{
			_current = n;

			if ( _box.ptr() )
			{
				_box->MoveCurrent( n );
			}
		}

		if ( saved != _current && notify )
		{
			OnItemChanged( _current );
		}
	}
	
	void ComboBox::OnItemChanged( int ItemIndex )
	{
		if ( ItemIndex < 0 )
		{
			if ( IsReadOnly() )
			{
				_edit.Clear();
			}
		}
		else if ( ItemIndex < _list.count())
		{
			_edit.SetText( _list[ItemIndex].text.data() );
		}
		
		Command( CMD_ITEM_CHANGED, ItemIndex, this, 0 );
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
		   _edit( 0, this, 0, 0, cols, false, EditLine::USEPARENTFOCUS | ( ( flags & READONLY ) ? EditLine::READONLY : 0 ) ),
		   _rows( rows > 0 ? rows : 0 )
	{
		_lo.AddWin( &_edit, 1, 1 );
		_edit.SetTabFocusFlag( false );
		_edit.SetShowSpaces( false );
		_edit.Enable();
		_edit.Show( SHOW_INACTIVE );
		
		_lo.AddRect( &_buttonRect, 1, 2 );
		_lo.ColSet( 2, CB_BUTTONWIDTH );
		
		const int fw = ( _flags & FRAME3D ) != 0 ? 3 : ( ( _flags & NOFOCUSFRAME ) ? 0 : 1 );
		_lo.ColSet( 0, fw );
		_lo.ColSet( 3, fw );
		_lo.LineSet( 0, fw );
		_lo.LineSet( 2, fw );
		SetLayout( &_lo );
	}

	void ComboBox::Clear()
	{
		if ( _box.ptr() )
		{
			_box->Clear();
		}

		_list.clear();

		if ( _current >= 0 )
		{
			_current = -1;

			if ( IsVisible() )
			{
				Invalidate();
			}
		}
	}

	void ComboBox::Append( const unicode_t* text, void* data )
	{
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
		return _edit.GetText();
	}

	std::string ComboBox::GetTextStr() const
	{
		std::vector<unicode_t> V = GetText();

		return unicode_to_utf8_string( V.data() );
	}

	void ComboBox::SetText( const unicode_t* txt, bool mark )
	{
		if ( !IsReadOnly() )
		{
			_edit.SetText( txt, mark );
		}
	}

	void ComboBox::SetText( const std::string& utf8txt, bool mark )
	{
		SetText( utf8str_to_unicode( utf8txt ).data(), mark );
	}

	void ComboBox::InsertText( unicode_t t )
	{
		if ( !IsReadOnly() )
		{
			_edit.Insert( t );
		}
	}

	void ComboBox::InsertText( const unicode_t* txt )
	{
		if ( !IsReadOnly() )
		{
			_edit.Insert( txt );
		}
	}

	void* ComboBox::ItemData( int n )
	{
		return ( n >= 0 && n < _list.count() ) ? _list[n].data : 0;
	}

	bool ComboBox::Command( int id, int subId, Win* win, void* d )
	{
		if ( win == _box.ptr() && _box.ptr() )
		{
			if ( !IsReadOnly() )
			{
				const int n = _box->GetCurrent();
				if ( n >= 0 && n < _list.count() && n != _current )
				{
					_current = n;
					OnItemChanged( _current );
				}
			}
			
			if ( id == CMD_ITEM_CLICK )
			{
				_current = _box->GetCurrent();
				OnItemChanged( _current );
				CloseBox();
			}

			return true;
		}

		if ( win == &_edit && _current != -1 )
		{
			_current = -1;
			if ( _box )
			{
				_box->SetNoCurrent();
			}

			OnItemChanged(_current);
			return true;
		};

		return Win::Command( id, subId, win, d );
	}

	bool ComboBox::EventFocus( bool recv )
	{
		_edit.EventFocus( recv );

		if ( !recv && IsBoxOpened() )
		{
			CloseBox();
		}

		Invalidate();
		return true;
	}

	bool ComboBox::EventKey( cevent_key* pEvent )
	{
		if ( pEvent->Type() == EV_KEYDOWN )
		{
			const bool ctrl = (pEvent->Mod() & KM_CTRL) != 0;

			switch ( pEvent->Key() )
			{
			case VK_ESCAPE:
				if ( IsBoxOpened() )
				{
					CloseBox();
					return true;
				}

				break;

			case VK_RETURN:
				if ( IsBoxOpened() )
				{
					const int n = _box->GetCurrent();
					_current = n;
					OnItemChanged( _current );
					CloseBox();
					return true;
				}

				break;

			case VK_UP:
				if ( ctrl && !IsBoxOpened() )
				{
					OpenBox();
					return true;
				}

				if ( IsBoxOpened() )
				{
					return _box->EventKey( pEvent );
				}

				break;

			case VK_DOWN:
				if ( ctrl && !IsBoxOpened() )
				{
					OpenBox();
					return true;
				}

				if ( IsBoxOpened() )
				{
					return _box->EventKey( pEvent );
				}

				break;
					
			default:
				if ( _edit.EventKey( pEvent ) )
				{
					return true;
				}

				if ( IsBoxOpened() && _box->EventKey( pEvent ) )
				{
					return true;
				}
				
				break;
			}
		}

		return Win::EventKey( pEvent );
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
		if ( !_box.ptr() )
		{
			return;
		}

		_box->Clear();

		for ( int i = 0; i < _list.count(); i++ )
		{
			_box->Append( _list[i].text.data() );
		}

		_box->DataRefresh();
	}

	void ComboBox::OpenBox()
	{
		if ( _box.ptr() )
		{
			return;
		}

		if ( !OnOpenBox() )
		{
			return;
		}

		_box = new TextList( Win::WT_POPUP, 0, 0, this, VListWin::SINGLE_SELECT, VListWin::SINGLE_BORDER, 0 );

		for ( int i = 0; i < _list.count(); i++ )
		{
			_box->Append( _list[i].text.data() );
		}

		if ( _current >= 0 )
		{
			_box->MoveCurrent( _current );
		}

		_box->SetHeightRange( LSRange( _rows, _rows, _rows ) );

		int width = ClientRect().Width();
		if ( width < 20 )
		{
			width = 20;
		}

		_box->Show( Win::SHOW_INACTIVE );
		_box->Enable();

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
		SetCapture( &captureSD );
	}

	void ComboBox::CloseBox()
	{
		if ( IsCaptured() )
		{
			ReleaseCapture( &captureSD );
		}

		if ( _box.ptr() )
		{
			OnCloseBox();
			_box = 0;
		}
	}

	bool ComboBox::EventMouse( cevent_mouse* pEvent )
	{
		crect ScrRect = ScreenRect();
		cpoint ScrPoint = pEvent->Point();
		ScrPoint.x += ScrRect.left;
		ScrPoint.y += ScrRect.top;

		if ( pEvent->Type() == EV_MOUSE_PRESS 
			&& (_buttonRect.In( pEvent->Point() ) || (IsReadOnly() && ScrRect.In( ScrPoint ))) )
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
			crect EditRect = _edit.ScreenRect();

			if ( EditRect.In( ScrPoint ) )
			{
				cevent_mouse ev( pEvent->Type(),
									  cpoint( ScrPoint.x - EditRect.left, ScrPoint.y - EditRect.top ),
									  pEvent->Button(),
									  pEvent->ButtonFlag(),
									  pEvent->Mod() );
				return _edit.EventMouse( &ev );
			}
			
			if ( _box.ptr() )
			{
				crect BoxRect = _box->ScreenRect();

				if ( BoxRect.In( ScrPoint ) )
				{
					cevent_mouse ev( pEvent->Type(),
										  cpoint( ScrPoint.x - BoxRect.left, ScrPoint.y - BoxRect.top ),
										  pEvent->Button(),
					                 pEvent->ButtonFlag(),
					                 pEvent->Mod() );
					return _box->EventMouse( &ev );
				}
				
				if ( pEvent->Type() == EV_MOUSE_PRESS )
				{
					CloseBox();
				}
			}

			return true;
		}

		return Win::EventMouse( pEvent );
	}


	extern void SBCDrawButton( GC& gc, crect rect, int type, unsigned bg, bool pressed );


	void ComboBox::Paint( GC& gc, const crect& paintRect )
	{
		crect cr = ClientRect();
		unsigned bgColor = UiGetColor( uiBackground, 0, 0, 0xD8E9EC );
		
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
		}

		DrawBorder( gc, cr, InFocus() && ( _flags & NOFOCUSFRAME ) == 0 ? 
            UiGetColor( uiFocusFrameColor, 0, 0, 0 ) : ColorTone( UiGetColor( uiFrameColor, 0, 0, 0xFFFFFF ), -200 ) );

		SBCDrawButton( gc, _buttonRect, ( _flags & MODE_UP ) ? 4 : 5, bgColor, false );
	}

} //namespace wal
