/*
* Part of Wal Commander GitHub Edition
* https://github.com/corporateshark/WalCommander
* walcommander@linderdaum.com
*/

#include "nceditline.h"
#include "wcm-config.h"
#include "string-util.h"
#include "unicode_lc.h"


typedef ccollect<std::vector<unicode_t>> HistCollect;

static cstrhash<clPtr<HistCollect>> histHash;


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

std::vector<char> HistSectionName( const char* fieldName )
{
	return carray_cat<char>( "history-field-", fieldName );
}

HistCollect* HistGetList( const char* fieldName )
{
	HistCollect* list = new HistCollect;
	list->append( utf8_to_unicode( "test" ) );
	list->append( utf8_to_unicode( "test 2" ) );
	list->append( utf8_to_unicode( "test 3" ) );
	return list;

	if ( !fieldName || !fieldName[0] )
	{
		return 0;
	}

	if ( HistoryCanSave() )
	{
		std::vector<std::string> list;
		clPtr<HistCollect> pUList = new HistCollect;
		LoadStringList( HistSectionName( fieldName ).data(), list );

		const int n = list.size();
		for ( int i = 0; i < n; i++ )
		{
			if ( list[i].data() && list[i][0] )
			{
				pUList->append( utf8_to_unicode( list[i].data() ) );
			}
		}

		histHash[fieldName] = pUList;
	}

	clPtr<HistCollect>* pp = histHash.exist( fieldName );
	if ( pp && pp->ptr() )
	{
		return pp->ptr();
	}

	clPtr<HistCollect> pUList = new HistCollect;
	HistCollect *pList = pUList.ptr();
	histHash[fieldName] = pUList;

	return pList;
}

bool HistStrcmp( const unicode_t* a, const unicode_t* b )
{
	while ( *a && *b && *a == *b )
	{
		a++;
		b++;
	}

	while ( *a && (*a == ' ' || *a == '\t') )
	{
		a++;
	}

	while ( *b && (*b == ' ' || *b == '\t') )
	{
		b++;
	}

	return (!*a && !*b);
}

void HistCommit( const char* fieldName, const unicode_t* txt )
{
	if ( !fieldName || !fieldName[0] )
	{
		return;
	}

	if ( !txt || !txt[0] )
	{
		return;
	}

	HistCollect *pList = HistGetList( fieldName );
	clPtr<HistCollect> pUList = new HistCollect;
	pUList->append( new_unicode_str( txt ) );

	int n = pList->count();
	for ( int i = 0; i < n; i++ )
	{
		if ( !HistStrcmp( txt, pList->get( i ).data() ) )
		{
			pUList->append( pList->get( i ) );
		}
	}

	pList = pUList.ptr();
	histHash[fieldName] = pUList;

	if ( HistoryCanSave() )
	{
		std::vector<std::string> list;
		int n = pList->count();

		if ( n > 50 )
		{
			n = 50; // limit amount of saved data
		}

		for ( int i = 0; i < n; i++ )
		{
			list.push_back( unicode_to_utf8_string( pList->get( i ).data() ) );
		}

		SaveStringList( HistSectionName( fieldName ).data(), list );
	}
}

static int uiClassNCEditLine = GetUiID( "NCEditLine" );

int NCEditLine::UiGetClassId()
{
	return uiClassNCEditLine;
}

NCEditLine::NCEditLine( const char* fieldName, int nId, Win* parent, const unicode_t* txt,
	int cols, int rows, bool up, bool frame3d, bool nofocusframe, crect* rect )
	: ComboBox( nId, parent, cols, rows, (up ? ComboBox::MODE_UP : 0) | (frame3d ? ComboBox::FRAME3D : 0) | (nofocusframe ? ComboBox::NOFOCUSFRAME : 0), rect )
	, m_fieldName( fieldName )
{
	clPtr<ccollect<std::vector<unicode_t>>> histList = 0;

	if ( m_fieldName && m_fieldName[0] )
	{
		histList = HistGetList( m_fieldName );
	}

	if ( histList.ptr() )
	{
		const int n = histList->count();
		for ( int i = 0; i < n; i++ )
		{
			const unicode_t *u = histList->get( i ).data();
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
		if ( (text.size() - 1) == cursorPos )
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
	HistCommit( m_fieldName, GetText().data() );
}
