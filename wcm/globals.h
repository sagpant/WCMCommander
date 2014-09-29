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

extern clPtr<cfont> panelFont;
extern clPtr<cfont> viewerFont;
extern clPtr<cfont> editorFont;
extern clPtr<cfont> dialogFont;
extern clPtr<cfont> terminalFont;
extern clPtr<cfont> helpTextFont;
extern clPtr<cfont> helpBoldFont;
extern clPtr<cfont> helpHeadFont;
