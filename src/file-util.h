/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * walcommander@linderdaum.com
 */

#pragma once

#include "globals.h"


template <class T> class clPtr;
class NCDialogParent;
class FS;
class FSPath;
class PanelWin;


/// Removes WCM Temp directory by the given Id
void RemoveWcmTempDir( const int Id );

/// Removes all created WCM Temp directories
void RemoveAllWcmTempDirs();

/// Loads to local temp file, returns local FS and Path back
int LoadToTempFile( NCDialogParent* parent, clPtr<FS>* fs, FSPath* path );

/// Opens HOME dir in the given panel
void OpenHomeDir( PanelWin* p, PanelWin* OtherPanel );

#ifdef _WIN32

/// Returns HOME dir URL on Win32
std::vector<unicode_t> GetHomeUriWin();

#endif
