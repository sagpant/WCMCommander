/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#pragma once

#include "vfs-smb.h"

#ifdef LIBSMBCLIENT_EXIST
bool GetSmbLogon( NCDialogParent* parent, FSSmbParam& params, bool enterServer = false );
#endif
