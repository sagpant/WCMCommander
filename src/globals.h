/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#pragma once

#include "swl/swl.h"
#include "wcm-config.h"
#include "fileassociations.h"
#include "filehighlighting.h"
#include "usermenu.h"

#include <vector>

// Windows
#define WIN_REGISTRY_FIRM  "Wal" //firm name in windows registry
#define WIN_REGISTRY_APP   "Wal commander" //app name in windows registry

// Unix
#ifndef UNIX_CONFIG_DIR_PATH
#	define UNIX_CONFIG_DIR_PATH  "/usr/share/wcm"
#endif

#if defined(_MSC_VER)
#	define UNUSED
#else
#	define UNUSED __attribute__ ((unused))
#endif

extern clWcmConfig g_WcmConfig;

extern clPtr<cfont> g_PanelFont;
extern clPtr<cfont> g_ViewerFont;
extern clPtr<cfont> g_EditorFont;
extern clPtr<cfont> g_DialogFont;
extern clPtr<cfont> g_TerminalFont;
extern clPtr<cfont> g_HelpTextFont;
extern clPtr<cfont> g_HelpBoldFont;
extern clPtr<cfont> g_HelpHeadFont;

extern bool g_DebugKeyboard;

class clEnvironment
{
public:
	const std::vector<clNCFileAssociation>& GetFileAssociations() const { return m_FileAssociations; }
	std::vector<clNCFileAssociation>* GetFileAssociationsPtr() { return &m_FileAssociations; }
	void SetFileAssociations( const std::vector<clNCFileAssociation>& Assoc ) { m_FileAssociations = Assoc; }

	const std::vector<clNCFileHighlightingRule>& GetFileHighlightingRules() const { return m_FileHighlightingRules; }
	std::vector<clNCFileHighlightingRule>* GetFileHighlightingRulesPtr() { return &m_FileHighlightingRules; }
	void SetFileHighlightingRules( const std::vector<clNCFileHighlightingRule>& Rules ) { m_FileHighlightingRules = Rules; }

	const std::vector<clNCUserMenuItem>& GetUserMenuItems() const { return m_UserMenuItems; }
	std::vector<clNCUserMenuItem>* GetUserMenuItemsPtr() { return &m_UserMenuItems; }
	void SetUserMenuItems( const std::vector<clNCUserMenuItem>& Items ) { m_UserMenuItems = Items; }

private:
	/// F2 user menu
	std::vector<clNCUserMenuItem> m_UserMenuItems;

	/// currently active file associations
	std::vector<clNCFileAssociation> m_FileAssociations;

	/// currently active file highlighting rules
	std::vector<clNCFileHighlightingRule> m_FileHighlightingRules;
};

extern clEnvironment g_Env;

class NCWin;

extern NCWin* g_MainWin;
