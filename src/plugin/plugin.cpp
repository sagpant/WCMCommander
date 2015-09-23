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
clArchPlugin g_ArchPlugin;


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

bool Plugin_OpenFileVFS( PanelWin* Panel, clPtr<FS> Fs, FSPath& Path )
{
	std::string FileExtLower = GetFileExt( std::string( Path.GetItem( Path.Count() - 1 )->GetUtf8() ) );
	std::transform(FileExtLower.begin(), FileExtLower.end(), FileExtLower.begin(), ::tolower);
	if ( FileExtLower.length() > 0 )
	{
		// trim dot char at the beginning
		FileExtLower = FileExtLower.substr( 1 );
	}
	
	clPtr<FS> Vfs;
	for ( auto& iter : clPluginFactory::s_Registry )
	{
		const clPluginFactory* PluginFactory = iter.second;

		Vfs = PluginFactory->OpenFileVFS( Fs, Path, FileExtLower );
		if ( Vfs.Ptr() != nullptr )
		{
			break;
		}
	}

	if ( Vfs.Ptr() == nullptr )
	{
		return false;
	}

	FSString RootPath = FSString( "/" );
	FSPath VfsPath = FSPath( RootPath );
	Panel->LoadPath( Vfs, VfsPath, nullptr, nullptr, PanelWin::PUSH );
	return true;
}
