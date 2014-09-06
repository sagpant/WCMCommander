#ifndef FONTDLG_H
#define FONTDLG_H
#include "ncdialogs.h"

clPtr<cfont> SelectX11Font( NCDialogParent* parent, bool fixed );
clPtr<cfont> SelectFTFont( NCDialogParent* parent, bool fixed, const char* currentUri );

#endif

