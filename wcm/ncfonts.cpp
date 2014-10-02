/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#include "globals.h"
#include "wcm-config.h"

inline void IFE( clPtr<cfont>* p, const char* fontUri )
{
	if ( !p->ptr() && fontUri && fontUri[0] )
	{
		*p = cfont::New( fontUri );
	}
}

void InitFonts()
{
	g_PanelFont = nullptr;
	g_ViewerFont = nullptr;
	g_EditorFont = nullptr;
	g_DialogFont = nullptr;
	g_TerminalFont = nullptr;
	g_HelpTextFont = nullptr;
	g_HelpBoldFont = nullptr;
	g_HelpHeadFont = nullptr;

	IFE( &g_PanelFont,   g_WcmConfig.panelFontUri.data() );
#ifdef _WIN32
	IFE( &g_PanelFont, "-100:Arial:B" );
#else
	IFE( &g_PanelFont, "-*-fixed-medium-r-*-*-*-140-*-*-*-*-iso10646-*" );
#endif

	IFE( &g_DialogFont, g_WcmConfig.dialogFontUri.data() );
#ifdef _WIN32
	IFE( &g_DialogFont, "-80:MS Sans Serif:B" );
#else
	IFE( &g_DialogFont, "-*-fixed-bold-r-*-*-*-120-*-*-*-*-iso10646-*" );
#endif


	IFE( &g_ViewerFont, g_WcmConfig.viewerFontUri.data() );
#ifdef _WIN32
	IFE( &g_ViewerFont, "-120:Lucida Console:FN" );
#else
	IFE( &g_ViewerFont, "-*-fixed-medium-r-*-*-*-140-*-*-*-*-iso10646-*" );
#endif


	IFE( &g_EditorFont, g_WcmConfig.editorFontUri.data() );
#ifdef _WIN32
	IFE( &g_EditorFont, "-120:Lucida Console:FN" );
#else
	IFE( &g_EditorFont, "-*-fixed-medium-r-*-*-*-140-*-*-*-*-iso10646-*" );
#endif

	IFE( &g_TerminalFont, g_WcmConfig.terminalFontUri.data() );
#ifdef _WIN32
	IFE( &g_TerminalFont, "-100:Lucida Console:FN" );
#else
	IFE( &g_TerminalFont, "-*-fixed-medium-r-*-*-*-140-*-*-*-*-iso10646-*" );
#endif

	IFE( &g_HelpTextFont, g_WcmConfig.helpTextFontUri.data() );
#ifdef _WIN32
	IFE( &g_HelpTextFont, "-100:Arial:N" );
#else
	IFE( &g_HelpTextFont, "-*-fixed-medium-r-*-*-*-120-*-*-*-*-iso10646-*" );
#endif

	IFE( &g_HelpBoldFont, g_WcmConfig.helpBoldFontUri.data() );
#ifdef _WIN32
	IFE( &g_HelpBoldFont, "-90:Arial:B" );
#else
	IFE( &g_HelpBoldFont, "-*-fixed-bold-r-*-*-*-120-*-*-*-*-iso10646-*" );
#endif

	IFE( &g_HelpHeadFont, g_WcmConfig.helpHeadFontUri.data() );
#ifdef _WIN32
	IFE( &g_HelpHeadFont, "-110:Arial:B" );
#else
	IFE( &g_HelpHeadFont, "-*-fixed-medium-r-*-*-*-120-*-*-*-*-iso10646-*" );
#endif
}


