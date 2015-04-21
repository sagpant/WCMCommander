/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * wcm@linderdaum.com
 */

#pragma once

#include <vector>

#include "System/Types.h"

#include "ncdialogs.h"

class PanelWin;

class clNCFileHighlightingRule
{
public:
	clNCFileHighlightingRule();

	const std::vector<unicode_t>& GetMask() const { return m_Mask; }
	const std::vector<unicode_t>& GetDescription() const { return m_Description; }
	bool IsMaskEnabled() const { return m_MaskEnabled; }
	uint64_t GetSizeMin() const { return m_SizeMin; }
	uint64_t GetSizeMax() const { return m_SizeMax; }
	uint64_t GetAttributesMask() const { return m_AttributesMask; }
	uint32_t GetColorNormal() const { return m_ColorNormal; }
	uint32_t GetColorNormalBackground() const { return m_ColorNormalBackground; }
	uint32_t GetColorSelected() const { return m_ColorSelected; }
	uint32_t GetColorSelectedBackground() const { return m_ColorSelectedBackground; }
	uint32_t GetColorUnderCursorNormal() const { return m_ColorUnderCursorNormal; }
	uint32_t GetColorUnderCursorNormalBackground() const { return m_ColorUnderCursorNormalBackground; }
	uint32_t GetColorUnderCursorSelected() const { return m_ColorUnderCursorSelected; }
	uint32_t GetColorUnderCursorSelectedBackground() const { return m_ColorUnderCursorSelectedBackground; }

	void SetMask( const std::vector<unicode_t>& S ) { m_Mask = S; }
	void SetDescription( const std::vector<unicode_t>& S ) { m_Description = S; }
	void SetMaskEnabled( bool Enabled ) { m_MaskEnabled = Enabled; }

	void SetSizeMin( uint64_t v ) { m_SizeMin = v; }
	void SetSizeMax( uint64_t v ) { m_SizeMax = v; }
	void SetAttributesMask( uint64_t v ) { m_AttributesMask = v; }
	void SetColorNormal( uint32_t c ) { m_ColorNormal = c; }
	void SetColorNormalBackground( uint32_t c ) { m_ColorNormalBackground = c; }
	void SetColorSelected( uint32_t c ) { m_ColorSelected = c; }
	void SetColorSelectedBackground( uint32_t c ) { m_ColorSelectedBackground = c; }
	void SetColorUnderCursorNormal( uint32_t c ) { m_ColorUnderCursorNormal = c; }
	void SetColorUnderCursorNormalBackground( uint32_t c ) { m_ColorUnderCursorNormalBackground = c; }
	void SetColorUnderCursorSelected( uint32_t c ) { m_ColorUnderCursorSelected = c; }
	void SetColorUnderCursorSelectedBackground( uint32_t c ) { m_ColorUnderCursorSelectedBackground = c; }

	bool IsRulePassed( const unicode_t* FileName, uint64_t FileSize, uint64_t Attributes ) const;

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

bool FileHighlightingDlg( NCDialogParent* Parent, std::vector<clNCFileHighlightingRule>* HighlightingRules, PanelWin* Panel );
