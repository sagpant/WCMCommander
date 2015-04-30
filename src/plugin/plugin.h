/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * wcm@linderdaum.com
 */

#pragma once

#include "globals.h"


/// Abstract plugin factory
class clPluginFactory
{
private:

	clPluginFactory( const clPluginFactory& ) = delete;
	clPluginFactory& operator=(const clPluginFactory&) = delete;

public:
	virtual ~clPluginFactory() { }

	static void Register( clPluginFactory* PluginFactory );
	static void Unregister( clPluginFactory* PluginFactory );

	/// Returns unique plugin id
	virtual const char* GetPluginId() const = 0;
};


// plugin internal interface

template <class T> class clPtr;
class FS;
class FSPath;

clPtr<FS> Plugin_OpenFileVFS( const unicode_t* FileExt, clPtr<FS> fs, FSPath& path );
