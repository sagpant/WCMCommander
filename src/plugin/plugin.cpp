/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * wcm@linderdaum.com
 */

#include "plugin.h"
#include "vfs.h"
#include "panel.h"

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

bool Plugin_OpenFileVFS( PanelWin* Panel, clPtr<FS> Fs, FSString& Uri )
{
	const unicode_t* FileExt = Uri.GetUnicode();
	clPtr<FS> Vfs;
	
	printf( "path: %s\n", Uri.GetUtf8() );

	for ( auto& iter : g_PluginRegistry )
	{
		const clPluginFactory* PluginFactory = iter.second;

		Vfs = PluginFactory->OpenFileVFS( FileExt, Fs, Uri );

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
