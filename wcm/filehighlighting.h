/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#pragma once

#include "types.h"
#include "ncdialogs.h"

#include <vector>

class clNCFileHighlightingRule
{
public:
	clNCFileHighlightingRule()
	{}

	bool IsRulePassed( const std::vector<unicode_t>& FileName, uint64_t FileSize, uint64_t Attributes ) const;

private:
	std::vector<unicode_t> m_Mask;

	uint64_t m_SizeMin;
	uint64_t m_SizeMax;
	uint64_t m_AttributesMask;

	// RGB colors
	uint32_t m_ColorNormal;
	uint32_t m_ColorNormalBackground;

	uint32_t m_ColorSelected;
	uint32_t m_ColorSelectedBackground;

	uint32_t m_ColorUnderCursorNormal;
	uint32_t m_ColorUnderCursorNormalBackground;

	uint32_t m_ColorUnderCursorSelected;
	uint32_t m_ColorUnderCursorSelectedBackground;
};

bool FileHighlightingDlg( NCDialogParent* Parent, std::vector<clNCFileHighlightingRule>* HighlightingRules );
