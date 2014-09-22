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

	const std::vector<unicode_t>& GetMask() const { return m_Mask; }
	const std::vector<unicode_t>& GetDescription() const { return m_Description; }
	bool IsMaskEnabled() const { return m_MaskEnabled; }

	void SetMask( const std::vector<unicode_t>& S ) { m_Mask = S; }
	void SetDescription( const std::vector<unicode_t>& S ) { m_Description = S; }
	void SetMaskEnabled( bool Enabled ) { m_MaskEnabled = Enabled; }

	bool IsRulePassed( const std::vector<unicode_t>& FileName, uint64_t FileSize, uint64_t Attributes ) const;

private:
	std::vector<unicode_t> m_Mask;
	std::vector<unicode_t> m_Description;

	bool m_MaskEnabled;

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
