/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#pragma once

#include "fileopers.h"
#include "ncdialogs.h"
#include "operwin.h"
#include <list>

CoreCommands SearchFile(clPtr<FS> f, FSPath p, NCDialogParent* parent, FSPath* retPath, std::list<FSPath>& foundItemsList);
