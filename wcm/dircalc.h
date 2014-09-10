/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#pragma once

#include "fileopers.h"
#include "ncdialogs.h"
#include "operwin.h"

bool DirCalc( clPtr<FS> f, FSPath& path, clPtr<FSList> list, NCDialogParent* parent );
