/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#pragma once

#include "ncdialogs.h"
#include "vfs.h"


void LoadFoldersHistory();

void SaveFoldersHistory();

void AddFolderToHistory(clPtr<FS>* fs, FSPath* path);

bool FolderHistoryDlg(NCDialogParent* parent, clPtr<FS>* fp, FSPath* pPath);
