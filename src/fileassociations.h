/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * wcm@linderdaum.com
 */

#pragma once

#include "ncdialogs.h"

#include <vector>

enum eFileAssociation
{
	eFileAssociation_Execute,
	eFileAssociation_ExecuteSecondary,
	eFileAssociation_View,
	eFileAssociation_ViewSecondary,
	eFileAssociation_Edit,
	eFileAssociation_EditSecondary,
};

class clNCFileAssociation
{
public:
	clNCFileAssociation()
		: m_HasTerminal( true )
	{}
	const std::vector<unicode_t>& GetMask() const { return m_Mask; }
	const std::vector<unicode_t>& GetDescription() const { return m_Description; }
	const std::vector<unicode_t>& GetExecuteCommand() const { return m_ExecuteCommand; }
	const std::vector<unicode_t>& GetExecuteCommandSecondary() const { return m_ExecuteCommandSecondary; }
	const std::vector<unicode_t>& GetViewCommand() const { return m_ViewCommand; }
	const std::vector<unicode_t>& GetViewCommandSecondary() const { return m_ViewCommandSecondary; }
	const std::vector<unicode_t>& GetEditCommand() const { return m_EditCommand; }
	const std::vector<unicode_t>& GetEditCommandSecondary() const { return m_EditCommandSecondary; }
	bool GetHasTerminal() const { return m_HasTerminal; }

	const std::vector<unicode_t>& Get( eFileAssociation Mode ) const
	{
		switch ( Mode )
		{
			case eFileAssociation_Execute:
				return GetExecuteCommand();

			case eFileAssociation_ExecuteSecondary:
				return GetExecuteCommandSecondary();

			case eFileAssociation_View:
				return GetViewCommand();

			case eFileAssociation_ViewSecondary:
				return GetViewCommandSecondary();

			case eFileAssociation_Edit:
				return GetEditCommand();

			case eFileAssociation_EditSecondary:
				return GetEditCommandSecondary();
		}

		return GetExecuteCommand();
	}

	void SetMask( const std::vector<unicode_t>& S ) { m_Mask = S; }
	void SetDescription( const std::vector<unicode_t>& S ) { m_Description = S; }
	void SetExecuteCommand( const std::vector<unicode_t>& S ) { m_ExecuteCommand = S; }
	void SetExecuteCommandSecondary( const std::vector<unicode_t>& S ) { m_ExecuteCommandSecondary = S; }
	void SetViewCommand( const std::vector<unicode_t>& S ) { m_ViewCommand = S; }
	void SetViewCommandSecondary( const std::vector<unicode_t>& S ) { m_ViewCommandSecondary = S; }
	void SetEditCommand( const std::vector<unicode_t>& S ) { m_EditCommand = S; }
	void SetEditCommandSecondary( const std::vector<unicode_t>& S ) { m_EditCommandSecondary = S; }
	void SetHasTerminal( bool HasTerminal ) { m_HasTerminal = HasTerminal; }

private:
	std::vector<unicode_t> m_Mask;
	std::vector<unicode_t> m_Description;
	std::vector<unicode_t> m_ExecuteCommand;
	std::vector<unicode_t> m_ExecuteCommandSecondary;
	std::vector<unicode_t> m_ViewCommand;
	std::vector<unicode_t> m_ViewCommandSecondary;
	std::vector<unicode_t> m_EditCommand;
	std::vector<unicode_t> m_EditCommandSecondary;
	bool m_HasTerminal;
};

bool FileAssociationsDlg( NCDialogParent* Parent, std::vector<clNCFileAssociation>* Associations );
