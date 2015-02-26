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
public:
	NCHistory(): m_List(), m_Current( 0 ) {}
	~NCHistory() {}

	void Clear();
	void Put( const unicode_t* str );
	void DeleteAll( const unicode_t* Str );

	size_t Count() const { return m_List.size(); }
	const unicode_t* operator[] ( size_t n ) { return n < m_List.size() ? m_List[n].data() : nullptr; }

	const unicode_t* Prev() { return ( m_Current < 0  || m_Current >= m_List.size() ) ? nullptr : m_List[m_Current++].data(); }
	const unicode_t* Next() { return ( m_Current == 0 || m_Current > m_List.size() ) ? nullptr : m_List[--m_Current].data(); }

	void ResetToLast() { m_Current = 0; }

private:
	std::vector< std::vector<unicode_t> > m_List;
	size_t m_Current;
};
