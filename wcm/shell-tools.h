#ifndef SHELL_TOOLS
#define SHELL_TOOLS
#include "ncdialogs.h"
#include "vfs.h"

carray<unicode_t> ShellTabKey( NCDialogParent* par, FSPtr fs, FSPath& path, const unicode_t* s, int* cursor ); //может вернуть 0 если ничего не сделано

#endif
