/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * wcm@linderdaum.com
 */

#include "plugin.h"
#include "vfs.h"
#include "panel.h"
#include "string-util.h"

#include <map>
#include <algorithm>
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
	
	for ( auto& iter : g_PluginRegistry )
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
