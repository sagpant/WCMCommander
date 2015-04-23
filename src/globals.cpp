/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * wcm@linderdaum.com
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
int  g_LoadCurrentDir = -1; // -1 = autodetect (i.e. load if launched from terminal), 0 = do not load, 1 = load

clEnvironment g_Env;

NCWin* g_MainWin = nullptr;
