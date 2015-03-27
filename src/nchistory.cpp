/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * walcommander@linderdaum.com
 */

#include "nchistory.h"
#include "wcm-config.h"


#define MAX_FIELD_HISTORY_COUNT	50

static const char fieldHistSection[] = "FieldsHistory";

static cstrhash<HistCollect_t> g_FieldsHistHash;


void LoadFieldsHistory()
{
	IniHash iniHash;
	IniHashLoad( iniHash, fieldHistSection );

	g_FieldsHistHash.clear();
	std::vector<const char*> SecList = iniHash.Keys();
	char Name[64];
	
	for ( int i = 0, count = SecList.size(); i < count; i++ )
	{
		const char* Section = SecList[i];
		HistCollect_t& List = g_FieldsHistHash.get( Section );

		for ( int j = 0; j < MAX_FIELD_HISTORY_COUNT; j++ )
		{
			Lsnprintf( Name, sizeof( Name ), "v%i", j );
			const char* Value = iniHash.GetStrValue( Section, Name, nullptr );
			if ( Value )
			{
				std::vector<unicode_t> Str = utf8_to_unicode( Value );
				List.push_back( Str );
			}
		}
	}
}

void SaveFieldsHistory()
{
	IniHash iniHash;
	std::vector<const char*> SecList = g_FieldsHistHash.keys();
	char Name[64];

	for ( int i = 0, SecCount = SecList.size(); i < SecCount; i++ )
	{
		const char* Section = SecList[i];
		HistCollect_t* List = g_FieldsHistHash.exist( Section );
		if ( !List )
		{
			continue;
		}

		for ( int j = 0, ValCount = List->size(); j < ValCount; j++ )
		{
			Lsnprintf( Name, sizeof( Name ), "v%i", j );
			std::string Val = unicode_to_utf8_string( List->at(j).data() );
			
			iniHash.SetStrValue( Section, Name, Val.c_str());
		}
	}

	IniHashSave( iniHash, fieldHistSection );
}

HistCollect_t* GetFieldHistCollect( const char* FieldName )
{
	if ( !FieldName || !FieldName[0] )
	{
		return nullptr;
	}
	
	HistCollect_t* List = g_FieldsHistHash.exist( FieldName );
	return List ? List : nullptr;
}

// Returns index of element with the specified text, or -1 if no such found
int FindHistElement( HistCollect_t* List, const unicode_t* Text )
{
	for (int i = 0, count = List->size(); i < count; i++)
	{
		if (unicode_is_equal( Text, List->at(i).data() ))
		{
			return i;
		}
	}
	
	return -1;
}

void AddFieldTextToHistory( const char* FieldName, const unicode_t* Txt )
{
	if ( !FieldName || !FieldName[0] || !Txt || !Txt[0] )
	{
		return;
	}
	
	std::vector<unicode_t> Str = new_unicode_str( Txt );
	
	HistCollect_t& List = g_FieldsHistHash.get( FieldName );
	
	// check if item already exists in the list
	const int Index = FindHistElement( &List, Str.data() );
	if ( Index != -1 )
	{
		// remove existing item
		List.erase( List.begin() + Index );
	}
	
	// add item to the begining of the list
	List.insert( List.begin(), Str );
	
	// limit number of elements in the list
	while ( List.size() > MAX_FIELD_HISTORY_COUNT )
	{
		List.erase( List.end() );
	}
}


void NCHistory::Clear()
{
	m_List.clear();
}

void NCHistory::DeleteAll( const unicode_t* Str )
{
	size_t i = 0;

	while ( i < m_List.size() )
	{
		if ( unicode_is_equal( Str, m_List[i].data() ) )
		{
			m_List.erase( m_List.begin() + i );
			continue;
		}

		i++;
	}

	// Maybe we need to keep m_Current pointing to specific command, if the latter survived?
	m_Current = -1;
}

void NCHistory::Put( const unicode_t* str )
{
	m_Current = -1;

	for ( size_t i = 0; i < m_List.size(); i++ )
	{
		const unicode_t* s = str;
		const unicode_t* t = m_List[i].data();

		while ( *t && *s )
		{
			if ( *t != *s ) { break; }

			s++;
			t++;
		}

		if ( *t == *s )
		{
			std::vector<unicode_t> p = m_List[i];
			m_List.erase( m_List.begin() + i );
			m_List.insert( m_List.begin(), p );
			return;
		}
	}

	const size_t MaxHistorySize = 1000;

	if ( m_List.size() > MaxHistorySize )
	{
		m_List.pop_back();
	}

	m_List.insert( m_List.begin(), new_unicode_str( str ) );
}

size_t NCHistory::Count() const
{
	return m_List.size();
}

const unicode_t* NCHistory::operator[] ( size_t n )
{
	return n < m_List.size() ? m_List[n].data() : nullptr;
}

const unicode_t* NCHistory::Prev()
{
    if ( !m_List.size() ) return nullptr;

	if ( m_Current >= (int) m_List.size()-1 )
	{
		m_Current = (int) m_List.size()-1;
		return m_List[m_Current].data();
	}
	
	return m_List[++m_Current].data();
}

const unicode_t* NCHistory::Next()
{
	if ( m_Current <= 0 )
	{
		m_Current = -1;
		return nullptr;
	}
	
	return ( m_Current == 0 || m_Current > (int) m_List.size() )
			? nullptr
			: m_List[--m_Current].data();
}
