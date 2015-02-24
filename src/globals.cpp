/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#include "globals.h"

clWcmConfig g_WcmConfig;

clPtr<cfont> g_PanelFont;
clPtr<cfont> g_ViewerFont;
clPtr<cfont> g_EditorFont;
clPtr<cfont> g_DialogFont;
clPtr<cfont> g_TerminalFont;
clPtr<cfont> g_HelpTextFont;
clPtr<cfont> g_HelpBoldFont;
clPtr<cfont> g_HelpHeadFont;

bool g_DebugKeyboard = false;

clEnvironment g_Env;

NCWin* g_MainWin = nullptr;
