/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#pragma once

#include "ncdialogs.h"

bool SelectCharset( NCDialogParent* parent, int* pCharsetId, int currentId = 0 );
bool SelectOperCharset( NCDialogParent* parent, int* pCharsetId, int currentId = 0 );

void InitOperCharsets();
int GetFirstOperCharsetId();
int GetNextOperCharsetId( int id );
