/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#pragma once

#include <wal.h>

using namespace wal;

class NCHistory
{
	ccollect< std::vector<unicode_t> > m_List;
	int m_Current;

public:
	NCHistory(): m_List(), m_Current( 0 ) {}

	void Clear();
	void Put( const unicode_t* str );
	void DeleteAll( const unicode_t* Str );

	int Count() const { return m_List.count(); }
	const unicode_t* operator[] ( int n ) { return n >= 0 && n < m_List.count() ? m_List[n].data() : 0; }

	unicode_t* Prev() { return ( m_Current < 0 || m_Current >= m_List.count() ) ? 0 : m_List[m_Current++].data(); }
	unicode_t* Next() { return ( m_Current <= 0 || m_Current > m_List.count() ) ? 0 : m_List[--m_Current].data(); }

	~NCHistory() {}
};
