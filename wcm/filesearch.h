#pragma once

#include "fileopers.h"
#include "ncdialogs.h"
#include "operwin.h"

bool SearchFile( clPtr<FS> f, FSPath p, NCDialogParent* parent, FSPath* retPath );
