/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * walcommander@linderdaum.com
 */

#ifdef _WIN32
#  include <winsock2.h>
#  include "resource.h"
#  include "w32util.h"
#else
#	include <limits.h>	// PATH_MAX
#	include <stdio.h>
#	include <stdlib.h>
#	include <libgen.h>	// dirname()
#endif

#include "nc.h"
#include "ncfonts.h"
#include "ncwin.h"
#include "panel.h"

#include <signal.h>
#include <locale.h>

#include "ext-app.h"
#include "wcm-config.h"
#include "color-style.h"
#include "vfs-sftp.h"
#include "charsetdlg.h"
#include "string-util.h"
#include "ltext.h"
#include "globals.h"
#include "eloadsave.h"
#include "file-util.h"

#include "internal_icons.inc"

clPtr<wal::GC> defaultGC;

const char* appName = "WCM Commander";
#if defined( _WIN32 )
const char* appNameRoot = "WCM Commander (Administrator)";
#else
const char* appNameRoot = "WCM Commander (Root)";
#endif

cfont* ( *OldSysGetFont )( Win* w, int id ) = 0;

cfont* MSysGetFont( Win* w, int id )
{
	if ( w )
	{
		if ( w->UiGetClassId() ==  uiClassPanel )
		{
			return g_PanelFont.ptr();
		}

		if ( w->UiGetClassId() == uiClassVListWin )
		{
			return g_PanelFont.ptr();
		}

		if ( w->UiGetClassId() == uiClassTerminal )
		{
			return g_TerminalFont.ptr();
		}

		if ( w->UiGetClassId() == uiClassEditor )
		{
			return g_EditorFont.ptr();
		}

		if ( w->UiGetClassId() == uiClassViewer )
		{
			return g_ViewerFont.ptr();
		}

		if (  w->UiGetClassId() == uiClassMenuBar ||
		      w->UiGetClassId() == uiClassPopupMenu ||
		      w->UiGetClassId() == uiClassToolTip ||
		      ( w->Parent() && w->UiGetClassId() == uiClassButton ) )
		{
			return g_DialogFont.ptr();
		}
	}

	return g_EditorFont.ptr();
}




#ifdef _WIN32
namespace wal
{
	extern size_t Win32HIconId;
	extern HINSTANCE Win32HInstance;
};
#endif

inline std::vector<sys_char_t> LTPath( const sys_char_t* fn, const char* ext )
{
	return carray_cat<sys_char_t>( fn, utf8_to_sys( ext ).data() );
}

static  bool InitLocale( const sys_char_t* dir, const char* id )
{
	if ( id[0] == '-' ) { return true; }

	std::vector<sys_char_t> fn =
#ifdef _WIN32
	   carray_cat<sys_char_t>( dir, utf8_to_sys( "\\ltext." ).data() );
#else
	   carray_cat<sys_char_t>( dir, utf8_to_sys( "/ltext." ).data() );
#endif

	if ( id[0] != '+' )
	{
		return LTextLoad( LTPath( fn.data(), id ).data() );
	}

	return LTextLoad( LTPath( fn.data(), sys_locale_lang_ter() ).data() ) ||
	       LTextLoad( LTPath( fn.data(), sys_locale_lang() ).data() );
};

extern const char* verString;

bool StartsWith( const char* s, const char* substr )
{
	int n = strlen( substr );

	return strncmp( s, substr, n ) == 0;
}

void ShowHelp()
{
	printf( "%s\n", verString );
	printf( "Copyright (C) Valery Goryachev 2013-2014, Copyright (C) Sergey Kosarevsky, 2014-2015\n" );
	printf( "http://wcm.linderdaum.com\n" );
	printf( "\nUsage: wcm [switches]\n\n" );
	printf( "The following switches may be used in the command line:\n\n" );
	printf( " /?\n" );
	printf( " -h\n" );
	printf( " --help                              This help.\n\n" );
#ifndef _WIN32
	printf( " --dlg                               Show dialogs as child windows (OS X/Linux only).\n\n" );
	printf( " --debug-keyboard                    Write keypresses to stderr (OS X/Linux only).\n\n" );
#endif
	printf( " /e [<line>:<pos>] <filename>\n" );
	printf( " -e [<line>:<pos>] <filename>\n" );
	printf( " --edit [<line>:<pos>] <filename>    Edit the specified file.\n\n" );

	printf( " /v <filename>\n" );
	printf( " -v <filename>\n" );
	printf( " --view <filename>                   View the specified file.\n\n" );
}

class iCommandlet: public iIntrusiveCounter
{
public:
	virtual ~iCommandlet() {};
	virtual bool Run() = 0;
};

class clCommandlet_EditFile: public iCommandlet
{
public:
	clCommandlet_EditFile( const char* FileName, int Line, int Pos, NCWin* NcWin )
		: m_FileName( FileName )
		, m_Line( Line )
		, m_Pos( Pos )
		, m_NCWin( NcWin )
	{}
	virtual bool Run() override
	{
		if ( !m_NCWin ) { return false; }

		std::vector<unicode_t> uri = utf8_to_unicode( m_FileName );

		bool Result = m_NCWin->StartEditor( uri, m_Line, m_Pos );

		if ( !Result )
		{
			printf( "Failed to start editor for %s\n", m_FileName );
			return false;
		}

		return Result;
	}

private:
	const char* m_FileName;
	int         m_Line;
	int         m_Pos;
	NCWin*      m_NCWin;
};

class clCommandlet_ViewFile: public iCommandlet
{
public:
	clCommandlet_ViewFile( const char* FileName, int Line, NCWin* NcWin )
		: m_FileName( FileName )
		, m_Line( Line )
		, m_NCWin( NcWin )
	{}
	virtual bool Run() override
	{
		if ( !m_NCWin ) { return false; }

		std::vector<unicode_t> uri = utf8_to_unicode( m_FileName );

		bool Result = m_NCWin->StartViewer( uri, m_Line );

		if ( !Result )
		{
			printf( "Failed to start viewer for %s\n", m_FileName );
			return false;
		}

		return Result;
	}

private:
	const char* m_FileName;
	int         m_Line;
	NCWin*      m_NCWin;
};

std::vector< clPtr<iCommandlet> > g_Applets;

bool ConvertToLinePos( const char* s, int* Line, int* Pos )
{
	if ( !Line || !Pos ) { return false; }

	*Line = 0;
	*Pos  = 0;

	int L, P;

	int NumRead = Lsscanf( s, "%i:%i", &L, &P );

	if ( NumRead != 2 ) { return false; }

	*Line = L;
	*Pos  = P;

	return true;
}

#define FETCH_ARG_AND_CHECK( i, msg ) i++; if ( i >= argc ) { printf( msg ); return false; }

bool ParseCommandLine( int argc, char** argv, NCWin* NcWin )
{
	int i = 1;

	for ( ; i < argc; i++ )
	{
		if ( !strcmp( argv[i], "--help" ) || !strcmp( argv[i], "-h" ) || !strcmp( argv[i], "/?" ) )
		{
			ShowHelp();
			return false;
		}

#ifndef _WIN32

		if ( !strcmp( argv[i], "--dlg" ) )
		{
			createDialogAsChild = false;
			continue;
		}

#endif

		if ( !strcmp( argv[i], "--debug-keyboard" ) )
		{
			g_DebugKeyboard = true;
			continue;
		}

		if ( !strcmp( argv[i], "--edit" ) || !strcmp( argv[i], "-e" ) || !strcmp( argv[i], "/e" ) )
		{
			FETCH_ARG_AND_CHECK( i, "Expected file name to edit" );

			int Line = 0;
			int Pos  = 0;

			if ( ConvertToLinePos( argv[i], &Line, &Pos ) )
			{
				FETCH_ARG_AND_CHECK( i, "Expected file name to edit" );
			}

			const char* FileName = argv[i];

			g_Applets.push_back( new clCommandlet_EditFile( FileName, Line, Pos, NcWin ) );

			continue;
		}

		if ( !strcmp( argv[i], "--view" ) || !strcmp( argv[i], "-v" ) || !strcmp( argv[i], "/v" ) )
		{
			FETCH_ARG_AND_CHECK( i, "Expected file name to view" );

			const char* FileName = argv[i];

			g_Applets.push_back( new clCommandlet_ViewFile( FileName, 0, NcWin ) );

			continue;
		}
	}

	return true;
}

#if defined( _WIN32 )
#pragma warning( disable:4996 ) // 'GetVersion' was declared deprecated
void SetHighDPIAware()
{
	DWORD dwVersion = 0;
	DWORD dwMajorVersion = 0;

	dwVersion = GetVersion();

	// Get the Windows version.
	dwMajorVersion = ( DWORD )( LOBYTE( LOWORD( dwVersion ) ) );

	if ( dwMajorVersion > 5 )
	{
		/// Anything below WinXP without SP3 does not support High DPI, so we do not enable it on WinXP at all.
		HMODULE Lib = ::LoadLibrary( "user32.dll" );

		typedef BOOL ( *SetProcessDPIAwareFunc )();

		SetProcessDPIAwareFunc SetDPIAware = ( SetProcessDPIAwareFunc )::GetProcAddress( Lib, "SetProcessDPIAware" );

		if ( SetDPIAware ) { SetDPIAware(); }

		FreeLibrary( Lib );
	}
}
#endif

std::string GetStartupDir( const char* ModulePath )
{
#if !defined(_WIN32)
	char Buf[ PATH_MAX ];
	
	const char* Res = realpath( ModulePath, Buf );

	return Res ? dirname( Buf ) : std::string();
#else
	return std::string();
#endif
}

#ifdef _WIN32
int APIENTRY _tWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR    lpCmdLine, int       nCmdShow )
#else
int main( int argc, char** argv )
#endif
{
	try
	{
#ifdef _WIN32
		SetHighDPIAware();
		//disable system critical messageboxes (for example: no floppy disk :) )
		SetErrorMode( SEM_FAILCRITICALERRORS );

		Win32HInstance = hInstance;
		Win32HIconId = IDI_WCM;

		WSADATA wsaData;

		if ( WSAStartup( MAKEWORD( 2, 2 ), &wsaData ) ) { throw_syserr( 0, "WSAStartup failed" ); }

		std::string StartupDir;
#else
		setlocale( LC_ALL, "" );
		signal( SIGPIPE, SIG_IGN );

		std::string StartupDir = GetStartupDir( argv[0] );
#endif

		try
		{
			InitConfigPath();
			g_WcmConfig.Load( nullptr, StartupDir );

			const char* langId = g_WcmConfig.systemLang.data() ? g_WcmConfig.systemLang.data() : "+";
#ifdef _WIN32
			if ( !InitLocale( L"install-files/share/wcm/lang", langId ) )
			{
				InitLocale( carray_cat<sys_char_t>( GetAppPath().data(), utf8_to_sys( "lang" ).data() ).data(), langId );
			}
#else

			if ( !InitLocale( "install-files/share/wcm/lang", langId ) )
			{
				InitLocale( UNIX_CONFIG_DIR_PATH "/lang", langId );
			}

#endif


//			SetEditorColorStyle(g_WcmConfig.editColorMode);
//			SetViewerColorStyle(g_WcmConfig.viewColorMode);
		}
		catch ( cexception* ex )
		{
			fprintf( stderr, "%s\n", ex->message() );
			ex->destroy();
		}

		InitSSH();

		AppInit();

		SetColorStyle( g_WcmConfig.styleColorTheme);

		OldSysGetFont = SysGetFont;
		SysGetFont = MSysGetFont;

//		cfont* defaultFont = SysGetFont( 0, 0 );
		defaultGC = new wal::GC( ( Win* )0 );

#ifndef _WIN32
#include "icons/wcm.xpm"
		Win::SetIcon( wcm_xpm );
#endif

		InitFonts();
		InitOperCharsets();

		SetCmdIcons();

		NCWin ncWin;

		g_MainWin = &ncWin;

		// reload config with a valid NCWin
		g_WcmConfig.Load( &ncWin, StartupDir );

		ncWin.Enable();
		ncWin.Show();

		InitExtensionApp();

#if !defined( _WIN32 )

		// don't bother with this on Windows
		if ( !ParseCommandLine( argc, argv, &ncWin ) ) { return 0; }

#endif

		for ( auto i = g_Applets.begin(); i != g_Applets.end(); i++ )
		{
			if ( !( *i )->Run() ) { return 0; }
		}

		g_Applets.clear();

		AppRun();

		// clean up
		RemoveAllWcmTempDirs();

		dbg_printf( "App Quit!!!\n" );

	}
	catch ( cexception* ex )
	{
#ifdef _WIN32
		MessageBoxA( 0, ex->message(), "", MB_OK );
#else
		printf( "Error: %s\n", ex->message() );
#endif
	}


	return 0;
}
