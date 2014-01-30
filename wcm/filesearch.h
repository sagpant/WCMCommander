#ifndef FILESEARCH_H
#define FILESEARCH_H

#include "fileopers.h"
#include "ncdialogs.h"
#include "operwin.h"

bool SearchFile(FSPtr f, FSPath p, NCDialogParent *parent, FSPath *retPath);

#endif
