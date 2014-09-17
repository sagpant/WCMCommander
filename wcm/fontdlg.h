/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#pragma once

#include "ncdialogs.h"

clPtr<cfont> SelectX11Font( NCDialogParent* parent, bool fixed );
clPtr<cfont> SelectFTFont( NCDialogParent* parent, bool fixed, const char* currentUri );
