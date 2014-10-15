/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#pragma once

#include "wal/wal.h"
#include "unicode_lc.h"

#include <vector>

bool accmask_nocase_begin( const unicode_t* name, const unicode_t* mask );
bool accmask( const unicode_t* name, const unicode_t* mask );

class clMultimaskSplitter
{
public:
	explicit clMultimaskSplitter( const std::vector<unicode_t>& MultiMask );

	bool HasNextMask() const;
	std::vector<unicode_t> GetNextMask();

	bool CheckAndFetchAllMasks( const unicode_t* FileName );
	bool CheckAndFetchAllMasks_NoCase( const unicode_t* FileName );
	bool CheckAndFetchAllMasks_Case( const unicode_t* FileName );

private:
	const std::vector<unicode_t>& m_MultiMask;
	size_t m_CurrentPos;
};
