/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#pragma once

#include "ncdialogs.h"

#include <vector>

class clNCFileHighlightingRule
{
public:
	clNCFileHighlightingRule()
	{}

	bool IsRulePassed( const std::vector<unicode_t>& FileName, uint64 FileSize, uint64 Attributes ) const;

private:
	std::vector<unicode_t> m_Mask;
	uint64 m_SizeMin;
	uint64 m_SizeMax;
	uint64 m_AttributesMask;
	// RGB colors
	uint32 m_ColorNormal;
	uint32 m_ColorSelected;
	uint32 m_ColorUnderCursorNormal;
	uint32 m_ColorUnderCursorSelected;
};

bool FileHighlightingDlg( NCDialogParent* Parent, std::vector<clNCFileHighlightingRule>* HighlightingRules );
