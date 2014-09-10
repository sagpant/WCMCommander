#pragma once

#include "vfs-smb.h"

#ifdef LIBSMBCLIENT_EXIST
bool GetSmbLogon( NCDialogParent* parent, FSSmbParam& params, bool enterServer = false );
#endif
