#include "wcm-config.h"

cptr<cfont> panelFont;
cptr<cfont> viewerFont;
cptr<cfont> editorFont;
cptr<cfont> dialogFont;
cptr<cfont> terminalFont;
cptr<cfont> helpTextFont;
cptr<cfont> helpBoldFont;
cptr<cfont> helpHeadFont;

inline void IFE( cptr<cfont>* p, const char* fontUri )
{
	if ( !p->ptr() && fontUri && fontUri[0] )
	{
		*p = cfont::New( fontUri );
	}
}

void InitFonts()
{
	panelFont = 0;
	viewerFont = 0;
	editorFont = 0;
	dialogFont = 0;
	terminalFont = 0;
	helpTextFont = 0;
	helpBoldFont = 0;
	helpHeadFont = 0;

	IFE( &panelFont,   wcmConfig.panelFontUri.data() );
#ifdef _WIN32
	//IFE(&panelFont, "-100:Consola:B");
	IFE( &panelFont, "-100:Arial:B" );
#else
	IFE( &panelFont, "-*-fixed-medium-r-*-*-*-140-*-*-*-*-iso10646-*" );
#endif

	IFE( &dialogFont, wcmConfig.dialogFontUri.data() );
#ifdef _WIN32
	IFE( &dialogFont, "-80:MS Sans Serif:B" );
#else
	IFE( &dialogFont, "-*-fixed-bold-r-*-*-*-120-*-*-*-*-iso10646-*" );
#endif


	IFE( &viewerFont, wcmConfig.viewerFontUri.data() );
#ifdef _WIN32
	IFE( &viewerFont, "-120:Lucida Console:FN" );
#else
	IFE( &viewerFont, "-*-fixed-medium-r-*-*-*-140-*-*-*-*-iso10646-*" );
#endif


	IFE( &editorFont, wcmConfig.editorFontUri.data() );
#ifdef _WIN32
	IFE( &editorFont, "-120:Lucida Console:FN" );
#else
	IFE( &editorFont, "-*-fixed-medium-r-*-*-*-140-*-*-*-*-iso10646-*" );
#endif

	IFE( &terminalFont, wcmConfig.terminalFontUri.data() );
#ifdef _WIN32
	IFE( &terminalFont, "-100:Lucida Console:FN" );
#else
	IFE( &terminalFont, "-*-fixed-medium-r-*-*-*-140-*-*-*-*-iso10646-*" );
#endif

	IFE( &helpTextFont, wcmConfig.helpTextFontUri.data() );
#ifdef _WIN32
	IFE( &helpTextFont, "-100:Arial:N" );
#else
	IFE( &helpTextFont, "-*-fixed-medium-r-*-*-*-120-*-*-*-*-iso10646-*" );
#endif

	IFE( &helpBoldFont, wcmConfig.helpBoldFontUri.data() );
#ifdef _WIN32
	IFE( &helpBoldFont, "-90:Arial:B" );
#else
	IFE( &helpBoldFont, "-*-fixed-bold-r-*-*-*-120-*-*-*-*-iso10646-*" );
#endif

	IFE( &helpHeadFont, wcmConfig.helpHeadFontUri.data() );
#ifdef _WIN32
	IFE( &helpHeadFont, "-110:Arial:B" );
#else
	IFE( &helpHeadFont, "-*-fixed-medium-r-*-*-*-120-*-*-*-*-iso10646-*" );
#endif
}


