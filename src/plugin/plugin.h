/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * wcm@linderdaum.com
 */

#pragma once

#include "globals.h"

#include <unordered_map>


template <class T> class clPtr;
class FS;
class FSString;
class FSPath;
struct FSNode;


/// plugin internal interface

clPtr<FS> Plugin_OpenFS( clPtr<FS> Fs, FSPath Path, const char* Name );


/// Abstract plugin factory
class clPluginFactory
{
	CLASS_COPY_PROTECTION( clPluginFactory );

public:
	static std::unordered_map<std::string, clPluginFactory*> s_Registry;

public:
	clPluginFactory() {}
	virtual ~clPluginFactory() { }

	static void Register( clPluginFactory* PluginFactory );
	static void Unregister( clPluginFactory* PluginFactory );

	/// Returns unique plugin id
	virtual const char* GetPluginId() const = 0;

	virtual clPtr<FS> OpenFS( clPtr<FS> Fs, FSPath& Path ) const = 0;
};
