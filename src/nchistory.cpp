/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#include "nchistory.h"

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
