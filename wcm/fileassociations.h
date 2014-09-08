#pragma once

#include "ncdialogs.h"

#include <vector>

class clNCFileAssociation
{
public:
	const std::vector<unicode_t>& GetMask() const { return m_Mask; }
	const std::vector<unicode_t>& GetDescription() const { return m_Description; }
	const std::vector<unicode_t>& GetExecuteCommand() const { return m_ExecuteCommand; }
	const std::vector<unicode_t>& GetExecuteCommandSecondary() const { return m_ExecuteCommandSecondary; }
	const std::vector<unicode_t>& GetViewCommand() const { return m_ViewCommand; }
	const std::vector<unicode_t>& GetViewCommandSecondary() const { return m_ViewCommandSecondary; }
	const std::vector<unicode_t>& GetEditCommand() const { return m_EditCommand; }
	const std::vector<unicode_t>& GetEditCommandSecondary() const { return m_EditCommandSecondary; }

	void SetMask( const std::vector<unicode_t>& S ) { m_Mask = S; }
	void SetDescription( const std::vector<unicode_t>& S ) { m_Description = S; }
	void SetExecuteCommand( const std::vector<unicode_t>& S ) { m_ExecuteCommand = S; }
	void SetExecuteCommandSecondary( const std::vector<unicode_t>& S ) { m_ExecuteCommandSecondary = S; }
	void SetViewCommand( const std::vector<unicode_t>& S ) { m_ViewCommand = S; }
	void SetViewCommandSecondary( const std::vector<unicode_t>& S ) { m_ViewCommandSecondary = S; }
	void SetEditCommand( const std::vector<unicode_t>& S ) { m_EditCommand = S; }
	void SetEditCommandSecondary( const std::vector<unicode_t>& S ) { m_EditCommandSecondary = S; }

private:
	std::vector<unicode_t> m_Mask;
	std::vector<unicode_t> m_Description;
	std::vector<unicode_t> m_ExecuteCommand;
	std::vector<unicode_t> m_ExecuteCommandSecondary;
	std::vector<unicode_t> m_ViewCommand;
	std::vector<unicode_t> m_ViewCommandSecondary;
	std::vector<unicode_t> m_EditCommand;
	std::vector<unicode_t> m_EditCommandSecondary;
};

bool FileAssociationsDlg( NCDialogParent* parent );
