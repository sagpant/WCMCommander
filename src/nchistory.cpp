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
	int i = 0;

	while ( i < m_List.count() )
	{
		if ( unicode_is_equal( Str, m_List[i].data() ) )
		{
			m_List.del( i );
			continue;
		}

		i++;
	}
}

void NCHistory::Put( const unicode_t* str )
{
	m_Current = 0;

	for ( int i = 0; i < m_List.count(); i++ )
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
			m_List.del( i );
			m_List.insert( 0, p );
			return;
		}
	}

	const size_t HistorySize = 1000;

	if ( m_List.count() > HistorySize )
	{
		m_List.del( m_List.count() - 1 );
	}

	m_List.insert( 0, new_unicode_str( str ) );
}
