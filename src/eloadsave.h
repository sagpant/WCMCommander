/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * walcommander@linderdaum.com
 */

#pragma once

#include "fileopers.h"
#include "ncdialogs.h"
#include "operwin.h"
#include "mfile.h"

clPtr<MemFile> LoadFile( clPtr<FS> f, FSPath& p, NCDialogParent* parent, bool ignoreENOENT );
bool SaveFile( clPtr<FS> f, FSPath& p, clPtr<MemFile> file, NCDialogParent* parent );
