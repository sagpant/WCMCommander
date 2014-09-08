#pragma once

#include "ncdialogs.h"

#include <vector>

class clNCFileAssociation
{
public:
	const std::vector<char>& GetMask() const { return m_Mask; }
	const std::vector<char>& GetEnterCommand() const { return m_EnterCommand; }
	void SetMask( const std::vector<char>& Mask ) { m_Mask = Mask; }
	void SetEnterCommand( const std::vector<char>& Cmd ) { m_EnterCommand = Cmd; }

private:
	std::vector<char> m_Mask;
	std::vector<char> m_EnterCommand;
};

bool FileAssociationsDlg( NCDialogParent* parent );
