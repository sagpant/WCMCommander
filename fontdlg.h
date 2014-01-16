#ifndef FONTDLG_H
#define FONTDLG_H
#include "ncdialogs.h"

cptr<cfont> SelectX11Font(NCDialogParent *parent, bool fixed);
cptr<cfont> SelectFTFont(NCDialogParent *parent, bool fixed, const char *currentUri);

#endif

