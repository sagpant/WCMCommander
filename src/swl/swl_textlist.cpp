/*
   Copyright (c) by Valery Goryachev (Wal)
*/


#include "swl.h"

namespace wal
{


	bool lessTLNode( TLNode* a, TLNode* b, void* )
	{
		unicode_t* sa = a->str.data(), *sb = b->str.data();

		while ( *sa && *sa == *sb ) { sa++; sb++; }

		return *sa < *sb;
	}

	int uiClassTextList = GetUiID( "TextList" );
	int TextList::UiGetClassId() { return uiClassTextList; }

	TextList::TextList( WTYPE tp, unsigned hints, int nId, Win* _parent, SelectType st, BorderType bt, crect* rect )
		: VListWin( tp, hints, nId, _parent, st, bt, rect ),
		  valid( false )
	{
		GC gc( this );
		gc.Set( GetFont() );
		cpoint ts = gc.GetTextExtents( ABCString );
		fontW = ( ts.x / ABCStringLen );
		fontH = ts.y + 2;
		this->SetItemSize( fontH + 1, GetItemWidth() ); //+1 for border if current
	}

	void TextList::DrawItem( GC& gc, int n, crect rect )
	{
		if ( n >= 0 && n < list.count() )
		{
			UiCondList ucl;

			if ( ( n % 2 ) == 0 ) { ucl.Set( uiOdd, true ); }

			if ( n == this->GetCurrent() ) { ucl.Set( uiCurrentItem, true ); }

			unsigned bg = UiGetColor( uiBackground, uiItem, &ucl, 0xFFFFFF );
			unsigned textColor = UiGetColor( uiColor, uiItem, &ucl, 0 );
			unsigned frameColor = UiGetColor( uiFrameColor, uiItem, &ucl, 0 );;

			bool frame = ( n == this->GetCurrent() );

			gc.SetFillColor( bg );
			gc.FillRect( rect );
			unicode_t* txt = list[n].str.data();

			if ( txt )
			{
				gc.Set( GetFont() );
				gc.SetTextColor( textColor );
				gc.SetFillColor( bg );
				gc.TextOutF( rect.left, rect.top + ( GetItemHeight() - fontH ) / 2, txt );
			}

			if ( frame )
			{
				DrawBorder( gc, rect, frameColor );
			}

		}
		else
		{
			gc.SetFillColor( UiGetColor( uiBackground, uiItem, 0, 0xFFFFFF ) );
			gc.FillRect( rect ); //CCC
		}
	}

	void TextList::Clear()
	{
		list.clear();
		valid = false;
		SetCount( list.count() );
	}


	void TextList::Append( const unicode_t* txt, int i, void* p )
	{
		TLNode node( txt, i, p );
		list.append( node );
		valid = false;
		SetCount( list.count() );
	}

	void TextList::DataRefresh()
	{
		if ( !valid )
		{
			valid = true;
			GC gc( this );
			gc.Set( GetFont() );
			int X = 0;

			int cnt = list.count();

			for ( int i = 0; i < cnt; i++ )
			{
				if ( list[i].pixelWidth < 0 )
				{
					list[i].pixelWidth = gc.GetTextExtents( list[i].str.data() ).x;
				}

				if ( list[i].pixelWidth > X )
				{
					X = list[i].pixelWidth;
				}
			}

			if ( GetCount() != list.count() )
			{
				SetCount( list.count() );
			}

			if ( X != this->GetItemWidth() )
			{
				SetItemSize( GetItemHeight(), X );
			}

			CalcScroll();
			Invalidate();
		}
	}

	void TextList::SetHeightRange( LSRange range )
	{
		LSize s = GetLSize();
		range.minimal *= fontH;
		range.maximal *= fontH;
		range.ideal *= fontH;
		s.y = range;
		SetLSize( s );
	}

	void TextList::SetWidthRange( LSRange range )
	{
		LSize s = GetLSize();
		range.minimal *= fontW;
		range.maximal *= fontW;
		range.ideal *= fontW;
		s.x = range;
		SetLSize( s );
	}

	TextList::~TextList() {}

}; // namespace wal
