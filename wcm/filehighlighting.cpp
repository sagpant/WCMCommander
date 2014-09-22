/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#include "filehighlighting.h"

bool clNCFileHighlightingRule::IsRulePassed( const std::vector<unicode_t>& FileName, uint64_t FileSize, uint64_t Attributes ) const
{
	return false;
}

bool FileHighlightingDlg( NCDialogParent* Parent, std::vector<clNCFileHighlightingRule>* HighlightingRules )
{
	return false;
}

