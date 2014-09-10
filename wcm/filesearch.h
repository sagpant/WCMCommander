/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#pragma once

#include "fileopers.h"
#include "ncdialogs.h"
#include "operwin.h"

bool SearchFile( clPtr<FS> f, FSPath p, NCDialogParent* parent, FSPath* retPath );
