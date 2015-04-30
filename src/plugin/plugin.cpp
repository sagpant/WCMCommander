/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * wcm@linderdaum.com
 */

#include "plugin.h"
#include "vfs.h"

#include <map>
#include <string>


static std::map<std::string, clPluginFactory*> g_PluginRegistry;


void clPluginFactory::Register( clPluginFactory* PluginFactory )
{
	g_PluginRegistry[std::string( PluginFactory->GetPluginId() )] = PluginFactory;
}

void clPluginFactory::Unregister( clPluginFactory* PluginFactory )
{
	g_PluginRegistry.erase( std::string( PluginFactory->GetPluginId() ) );
}

clPtr<FS> Plugin_OpenFileVFS( const unicode_t* FileExt, clPtr<FS> fs, FSPath& path )
{
	clPtr<FS> vfs;
	return vfs;
}
