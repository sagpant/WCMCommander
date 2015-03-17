/*
* Part of Wal Commander GitHub Edition
* https://github.com/corporateshark/WalCommander
* walcommander@linderdaum.com
*/

#include "nceditline.h"
#include "string-util.h"
#include "unicode_lc.h"
#include "nchistory.h"
#include "globals.h"


static int uiClassNCEditLine = GetUiID( "NCEditLine" );

int clNCEditLine::UiGetClassId()
{
	return uiClassNCEditLine;
}

clNCEditLine::clNCEditLine( const char* FieldName, int Id, Win* Parent, const unicode_t* Txt,
									int Cols, int Rows, bool Up, bool Frame3d, bool NoFocusFrame, crect* Rect )
	: ComboBox( Id, Parent, Cols, Rows,
				  (Up ? ComboBox::MODE_UP : 0) | (Frame3d ? ComboBox::FRAME3D : 0) | (NoFocusFrame ? ComboBox::NOFOCUSFRAME : 0),
				  Rect )
	, m_FieldName( FieldName )
{
	HistCollect_t* List = GetFieldHistCollect( m_FieldName );
	if ( List )
	{
		for ( int i = 0, count = List->size(); i < count; i++ )
		{
			const unicode_t* Str = List->at( i ).data();
			if ( Str )
			{
				Append( Str );
			}
		}
	}
	else
	{
		static unicode_t Str0 = 0;
		Append( &Str0 );
	}
}

bool AcEqual( const unicode_t* Txt, const unicode_t* Element, int Chars )
{
	if ( !Txt || !Element || Chars <= 0 )
	{
		return false;
	}

	while ( *Txt && *Element && UnicodeLC( *Txt ) == UnicodeLC( *Element ) )
	{
		if ( --Chars == 0 )
		{
			return true;
		}

		Txt++;
		Element++;
	}

	return false;
}

bool clNCEditLine::Command( int Id, int SubId, Win* Win, void* Data )
{
	if ( Id == CMD_EDITLINE_INFO && SubId == SCMD_EDITLINE_INSERTED && IsEditLine( Win ) && g_WcmConfig.systemAutoComplete )
	{
		std::vector<unicode_t> Text = GetText();
		const int CursorPos = GetCursorPos();
		
		// try to autocomplete when cursor is at the end of unmarked text
		if ( ((int)Text.size() - 1) == CursorPos )
		{
			const int count = Count();
			for ( int i = 0; i < count; i++ )
			{
				const unicode_t* Item = ItemText( i );
				if ( AcEqual( Text.data(), Item, CursorPos ) )
				{
					// append suffix of the history item's text to the current text
					SetText( carray_cat( Text.data(), Item + CursorPos ).data() );
					
					// select the auto-completed part of text
					SetCursorPos( carray_len( Item ) );
					SetCursorPos( CursorPos, true );
					return true;
				}
			}
		}
	}

	return ComboBox::Command( Id, SubId, Win, Data );
}

bool clNCEditLine::OnOpenBox()
{
	if ( Count() > 0 )
	{
		MoveCurrent( 0 );
	}
	
	return true;
}

void clNCEditLine::AddCurrentTextToHistory()
{
	AddFieldTextToHistory( m_FieldName, GetText().data() );
}
