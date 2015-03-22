/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#pragma once

#include "ncdialogs.h"
#include "vfs.h"


void LoadViewHistory();
void SaveViewHistory();

void AddFileToHistory( clPtr<FS>* fs, FSPath* path );

bool ViewHistoryDlg( NCDialogParent* parent, clPtr<FS>* fp, FSPath* pPath );
