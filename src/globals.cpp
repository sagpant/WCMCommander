/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * wcm@linderdaum.com
 */

#include "globals.h"
#include "panel.h"

// All globals, which are sensitive to initialization/destruction order, should be defined BELOW

#include "icons/folder3.xpm"
#include "icons/folder.xpm"
//#include "icons/executable.xpm"
#include "icons/executable1.xpm"
#include "icons/monitor.xpm"
#include "icons/workgroup.xpm"
#include "icons/run.xpm"
#include "icons/link.xpm"


namespace wal
{
	Mutex cicon::iconCopyMutex;
	Mutex cicon::iconListMutex;
}

cicon PanelWin::folderIcon(xpm16x16_Folder, PANEL_ICON_SIZE, PANEL_ICON_SIZE);
cicon PanelWin::folderIconHi(xpm16x16_Folder_HI, PANEL_ICON_SIZE, PANEL_ICON_SIZE);

cicon PanelWin::linkIcon(xpm16x16_Link, PANEL_ICON_SIZE, PANEL_ICON_SIZE);
cicon PanelWin::executableIcon(xpm16x16_Executable, PANEL_ICON_SIZE, PANEL_ICON_SIZE);
cicon PanelWin::serverIcon(xpm16x16_Monitor, PANEL_ICON_SIZE, PANEL_ICON_SIZE);
cicon PanelWin::workgroupIcon(xpm16x16_Workgroup, PANEL_ICON_SIZE, PANEL_ICON_SIZE);
cicon PanelWin::runIcon(xpm16x16_Run, PANEL_ICON_SIZE, PANEL_ICON_SIZE);

// All globals, which are sensitive to initialization/destruction order, should be defined ABOVE

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
