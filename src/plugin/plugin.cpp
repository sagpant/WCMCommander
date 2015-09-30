/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * wcm@linderdaum.com
 */

#include "plugin.h"
#include "vfs.h"
#include "panel.h"
#include "string-util.h"

#include <algorithm>
#include <string>

#include "plugin-archive.h"


std::unordered_map<std::string, clPluginFactory*> clPluginFactory::s_Registry;

//
// Register VFS plugins
//
#ifdef LIBARCHIVE_EXIST

clArchPlugin g_ArchPlugin;

#endif //LIBARCHIVE_EXIST


void clPluginFactory::Register( clPluginFactory* PluginFactory )
{
	std::string Id( PluginFactory->GetPluginId() );
	s_Registry[Id] = PluginFactory;
}

void clPluginFactory::Unregister( clPluginFactory* PluginFactory )
{
	std::string Id( PluginFactory->GetPluginId() );

	auto iter = s_Registry.find( Id );

	if ( iter != s_Registry.end() )
	{
		s_Registry.erase( iter );
	}
}

clPtr<FS> Plugin_OpenFS( clPtr<FS> Fs, FSPath Path, const char* Name )
{
	// get path with filename
	Path.Push( CS_UTF8, Name );

	for ( auto& iter : clPluginFactory::s_Registry )
	{
		const clPluginFactory* PluginFactory = iter.second;

		clPtr<FS> Vfs = PluginFactory->OpenFS( Fs, Path );

		if ( Vfs.Ptr() != nullptr )
		{
			return Vfs;
		}
	}

	return clPtr<FS>();
}
