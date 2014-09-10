#pragma once

#include "ncdialogs.h"

clPtr<cfont> SelectX11Font( NCDialogParent* parent, bool fixed );
clPtr<cfont> SelectFTFont( NCDialogParent* parent, bool fixed, const char* currentUri );
