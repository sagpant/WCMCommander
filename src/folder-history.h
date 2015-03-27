/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * walcommander@linderdaum.com
 */

#pragma once

#include "ncdialogs.h"
#include "vfs.h"


void LoadFoldersHistory();

void SaveFoldersHistory();

void AddFolderToHistory( clPtr<FS>* fs, FSPath* path );

bool FolderHistoryDlg( NCDialogParent* parent, clPtr<FS>* fp, FSPath* pPath );
