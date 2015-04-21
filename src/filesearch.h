/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * wcm@linderdaum.com
 */

#pragma once

#include "fileopers.h"
#include "ncdialogs.h"
#include "operwin.h"
#include <list>

CoreCommands SearchFile(clPtr<FS> f, FSPath p, NCDialogParent* parent, FSPath* retPath, std::list<FSPath>& foundItemsList);
