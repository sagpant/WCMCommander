/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * walcommander@linderdaum.com
 */

#pragma once

template <class T> class clPtr;
class NCDialogParent;
class FS;
class FSPath;

/// Loads to local temp file, returns local FS and Path back
bool LoadToTempFile( NCDialogParent* parent, clPtr<FS>* fs, FSPath* path );
