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

int NCEditLine::UiGetClassId()
{
	return uiClassNCEditLine;
}

NCEditLine::NCEditLine( const char* fieldName, int nId, Win* parent, const unicode_t* txt,
	int cols, int rows, bool up, bool frame3d, bool nofocusframe, crect* rect )
	: ComboBox( nId, parent, cols, rows,
				  (up ? ComboBox::MODE_UP : 0) | (frame3d ? ComboBox::FRAME3D : 0) | (nofocusframe ? ComboBox::NOFOCUSFRAME : 0),
				  rect )
	, m_fieldName( fieldName )
{
	HistCollect* pList = GetFieldHistCollect( m_fieldName );
	if ( pList )
	{
		for ( int i = 0, count = pList->size(); i < count; i++ )
		{
			const unicode_t* u = pList->at( i ).data();
			if ( u )
			{
				Append( u );
			}
		}
	}
	else
	{
		static unicode_t s0 = 0;
		Append( &s0 );
	}
}

bool AcEqual( const unicode_t* txt, const unicode_t* element, int chars )
{
	if ( !txt || !element || chars <= 0 )
	{
		return false;
	}

	while ( *txt && *element && UnicodeLC( *txt ) == UnicodeLC( *element ) )
	{
		if ( --chars == 0 )
		{
			return true;
		}

		txt++;
		element++;
	}

	return false;
}

bool NCEditLine::Command( int id, int subId, Win* win, void* d )
{
	if ( id == CMD_EDITLINE_INFO && subId == SCMD_EDITLINE_INSERTED && IsEditLine( win ) && g_WcmConfig.systemAutoComplete )
	{
		std::vector<unicode_t> text = GetText();
		const int cursorPos = GetCursorPos();
		
		// try to autocomplete when cursor is at the end of unmarked text
		if ( ((int)text.size() - 1) == cursorPos )
		{
			const int count = Count();
			for ( int i = 0; i < count; i++ )
			{
				const unicode_t* item = ItemText( i );
				if ( AcEqual( text.data(), item, cursorPos ) )
				{
					// append suffix of the history item's text to the current text
					SetText( carray_cat( text.data(), item + cursorPos ).data() );
					
					// select the auto-completed part of text
					SetCursorPos( carray_len( item ) );
					SetCursorPos( cursorPos, true );
					return true;
				}
			}
		}
	}

	return ComboBox::Command( id, subId, win, d );
}

bool NCEditLine::OnOpenBox()
{
	const int count = Count();
	if ( count > 0 )
	{
		MoveCurrent( 0 );
	}
	
	return true;
}

void NCEditLine::AddCurrentTextToHistory()
{
	AddFieldTextToHistory( m_fieldName, GetText().data() );
}
