/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * walcommander@linderdaum.com
 */

#pragma once

#include "ncdialogs.h"
#include "vfs.h"
#include "ncedit.h"


void LoadViewHistory();
void SaveViewHistory();

int GetCreateFileViewPosHistory( clPtr<FS>* Fs, FSPath* Path );
void UpdateFileViewPosHistory( const unicode_t* Name, const int Pos );

bool GetCreateFileEditPosHistory( clPtr<FS>* Fs, FSPath* Path, sEditorScrollCtx& Ctx );
void UpdateFileEditPosHistory( const unicode_t* Name, const sEditorScrollCtx& Ctx );

bool ViewHistoryDlg( NCDialogParent* Parent, clPtr<FS>* Fs, FSPath* Path, bool* IsEdit );
