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
	NCHistory(): m_List(), m_Current( -1 ) {}
	~NCHistory() {}

	void Clear();
	void Put( const unicode_t* str );
	void DeleteAll( const unicode_t* Str );

	size_t Count() const;
	const unicode_t* operator[] ( size_t n );

	const unicode_t* Prev();
	const unicode_t* Next();

	void ResetToLast() { m_Current = -1; }

private:
	std::vector< std::vector<unicode_t> > m_List;
	int m_Current;
};
