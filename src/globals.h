/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#pragma once

#include "swl/swl.h"
#include "wcm-config.h"

// Windows
#define WIN_REGISTRY_FIRM  "Wal" //firm name in windows registry
#define WIN_REGISTRY_APP   "Wal commander" //app name in windows registry

// Unix
#define UNIX_CONFIG_DIR_PATH  "/usr/share/wcm"

extern WcmConfig g_WcmConfig;

extern clPtr<cfont> g_PanelFont;
extern clPtr<cfont> g_ViewerFont;
extern clPtr<cfont> g_EditorFont;
extern clPtr<cfont> g_DialogFont;
extern clPtr<cfont> g_TerminalFont;
extern clPtr<cfont> g_HelpTextFont;
extern clPtr<cfont> g_HelpBoldFont;
extern clPtr<cfont> g_HelpHeadFont;

extern bool g_DebugKeyboard;
