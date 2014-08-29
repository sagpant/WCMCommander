#ifndef ELOADSAVE_H
#define ELOADSAVE_H

#include "fileopers.h"
#include "ncdialogs.h"
#include "operwin.h"
#include "mfile.h"

clPtr<MemFile> LoadFile( FSPtr f, FSPath& p, NCDialogParent* parent, bool ignoreENOENT );
bool SaveFile( FSPtr f, FSPath& p, clPtr<MemFile> file, NCDialogParent* parent );

#endif
