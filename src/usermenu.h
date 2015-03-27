/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * walcommander@linderdaum.com
 */

#pragma once

#include "ncdialogs.h"

#include <vector>

class clNCUserMenuItem
{
public:
	clNCUserMenuItem()
	{}
	const MenuTextInfo& GetDescription() const { return m_Description; }
	const std::vector<unicode_t>& GetCommand() const { return m_Command; }

	void SetDescription( const std::vector<unicode_t>& S ) { m_Description.SetText(&S[0]); }
	void SetCommand( const std::vector<unicode_t>& S ) { m_Command = S; }

private:
	MenuTextInfo m_Description;
	std::vector<unicode_t> m_Command;
};

bool UserMenuDlg( NCDialogParent* Parent, std::vector<clNCUserMenuItem>* MenuItems );
