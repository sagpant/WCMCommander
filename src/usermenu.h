/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
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
	const std::vector<unicode_t>& GetDescription() const { return m_Description; }
	const std::vector<unicode_t>& GetCommand() const { return m_Command; }

	void SetDescription( const std::vector<unicode_t>& S ) { m_Description = S; }
	void SetCommand( const std::vector<unicode_t>& S ) { m_Command = S; }

private:
	std::vector<unicode_t> m_Description;
	std::vector<unicode_t> m_Command;
};

bool UserMenuDlg( NCDialogParent* Parent, std::vector<clNCUserMenuItem>* MenuItems );
