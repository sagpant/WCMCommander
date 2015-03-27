/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * walcommander@linderdaum.com
 */

#pragma once

#include "vfs-smb.h"

#ifdef LIBSMBCLIENT_EXIST
bool GetSmbLogon( NCDialogParent* parent, FSSmbParam& params, bool enterServer = false );
#endif
