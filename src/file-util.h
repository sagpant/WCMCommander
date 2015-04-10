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


/// Removes WCM Temp directory by the given Id
void RemoveWcmTempDir( const int Id );

/// Loads to local temp file, returns local FS and Path back
int LoadToTempFile( NCDialogParent* parent, clPtr<FS>* fs, FSPath* path );
