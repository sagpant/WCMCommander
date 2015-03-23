/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#pragma once

#include "ncdialogs.h"
#include "vfs.h"
#include "ncedit.h"


void LoadViewHistory();
void SaveViewHistory();

int GetCreateFileViewPosHistory( clPtr<FS>* Fs, FSPath* Path );
void UpdateFileViewPosHistory( std::vector<unicode_t> Name, const int Pos );

bool GetCreateFileEditPosHistory( clPtr<FS>* Fs, FSPath* Path, sEditorScrollCtx& Ctx );
void UpdateFileEditPosHistory( std::vector<unicode_t> Name, const sEditorScrollCtx& Ctx );

bool ViewHistoryDlg( NCDialogParent* Parent, clPtr<FS>* Fs, FSPath* Path );
