/*
* Part of Wal Commander GitHub Edition
* https://github.com/corporateshark/WalCommander
* walcommander@linderdaum.com
*/

#include "nceditline.h"
#include "wcm-config.h"
#include "string-util.h"
#include "unicode_lc.h"


#define MAX_FIELD_HISTORY_COUNT	50

typedef ccollect<std::vector<unicode_t>> HistCollect;

static cstrhash<clPtr<HistCollect>> g_histHash;


inline bool HistoryCanSave()
{
	//return wcmConfig.systemSaveHistory;
	return true;
}

inline bool AcEnabled()
{
	//return wcmConfig.systemAutoComplete;
	return true;
}

HistCollect* HistGetList( const char* fieldName )
{
	if ( !fieldName || !fieldName[0] )
	{
		return nullptr;
	}
	
	clPtr<HistCollect>* pp = g_histHash.exist( fieldName );
	if ( pp && pp->ptr() )
	{
		return pp->ptr();
	}
	
	return nullptr;
}

// Returns index of element with the specified name, or -1 if no such found
int FindByName( HistCollect* pList, const unicode_t* name )
{
	for (int i = 0, count = pList->count(); i < count; i++)
	{
		if (unicode_is_equal( name, pList->get(i).data() ))
		{
			return i;
		}
	}
	
	return -1;
}

void AddFieldTextToHistory( const char* fieldName, const unicode_t* txt )
{
	if ( !HistoryCanSave() || !fieldName || !fieldName[0] || !txt || !txt[0] )
	{
		return;
	}

	std::vector<unicode_t> str = new_unicode_str( txt );

	// get history collection for field
	HistCollect* pList = HistGetList( fieldName );
	if ( !pList)
	{
		clPtr<HistCollect> pUList = new HistCollect;
		pList = pUList.ptr();
		g_histHash[fieldName] = pUList;
	}
	
	// check if item already exists in the list
	const int index = FindByName( pList, str.data() );
	if ( index != -1 )
	{
		// remove existing item
		pList->del( index );
	}
	
	// add item to the and of the list
	pList->insert( 0, str );
	
	// limit number of elements in the list
	while ( pList->count() > MAX_FIELD_HISTORY_COUNT )
	{
		pList->del( pList->count() - 1 );
	}
}

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
	HistCollect* pList = HistGetList( m_fieldName );
	if ( pList )
	{
		for ( int i = 0, count = pList->count(); i < count; i++ )
		{
			const unicode_t* u = pList->get( i ).data();
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
	if ( id == CMD_EDITLINE_INFO && subId == SCMD_EDITLINE_INSERTED && IsEditLine( win ) && AcEnabled() )
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
