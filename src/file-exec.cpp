/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * wcm@linderdaum.com
 */

#include "file-exec.h"
#include "file-util.h"
#include "ncwin.h"
#include "ltext.h"
#include "string-util.h"
#include "panel.h"
#include "strmasks.h"
#include "ext-app.h"

#ifndef _WIN32
#  include <signal.h>
#  include <sys/wait.h>
#  include "ux_util.h"
#else
#	include "w32util.h"
#endif


#define TERMINAL_THREAD_ID 1


void ReturnToDefaultSysDir()
{
#ifdef _WIN32
	wchar_t buf[4096] = L"";

	if ( GetSystemDirectoryW( buf, 4096 ) > 0 )
	{
		SetCurrentDirectoryW( buf );
	}

#else
	chdir( "/" );
#endif
}


enum
{
	CMD_RC_RUN = 999,
	CMD_RC_OPEN_0 = 1000
};

static const int CMD_OPEN_FILE = 1000;
static const int CMD_EXEC_FILE = 1001;


struct AppMenuData
{
	struct Node
	{
		unicode_t* cmd;
		bool terminal;
		Node() : cmd( 0 ), terminal( 0 ) {}
		Node( unicode_t* c, bool t ) : cmd( c ), terminal( t ) {}
	};
	
	ccollect<clPtr<MenuData>> mData;
	ccollect<Node> nodeList;
	MenuData* AppendAppList( AppList* list );
};

MenuData* AppMenuData::AppendAppList( AppList* list )
{
	if ( !list )
	{
		return 0;
	}

	clPtr<MenuData> p = new MenuData();

	for ( int i = 0; i < list->Count(); i++ )
	{
		if ( list->list[i].sub.ptr() )
		{
			MenuData* sub = AppendAppList( list->list[i].sub.ptr() );
			p->AddSub( list->list[i].name.data(), sub );
		}
		else
		{
			p->AddCmd( nodeList.count() + CMD_RC_OPEN_0, list->list[i].name.data() );
			nodeList.append( Node( list->list[i].cmd.data(), list->list[i].terminal ) );
		}
	}

	MenuData* ret = p.ptr();
	mData.append( p );
	return ret;
}


FileExecutor::FileExecutor( NCWin* NCWin, StringWin& editPref, NCHistory& history, TerminalWin_t& terminal )
	: m_NCWin( NCWin )
	, m_EditPref( editPref )
	, m_History( history )
	, m_Terminal( terminal )
	, m_ExecId( -1 )
{
	m_ExecSN[0] = 0;
}

void FileExecutor::ShowFileContextMenu( cpoint point, PanelWin* Panel )
{
	FSNode* p = Panel->GetCurrent();

	if ( !p || p->IsDir() )
	{
		return;
	}

	clPtr<AppList> appList = GetAppList( Panel->UriOfCurrent().GetUnicode() );

	//if (!appList.data()) return;

	AppMenuData data;
	MenuData mdRes, *md = data.AppendAppList( appList.ptr() );

	if ( !md )
	{
		md = &mdRes;
	}

	if ( p->IsExe() )
	{
		md->AddCmd( CMD_RC_RUN, _LT( "Execute" ) );
	}

	if ( !md->Count() )
	{
		return;
	}

	int ret = DoPopupMenu( 0, m_NCWin, md, point.x, point.y );

	m_NCWin->SetCommandLineFocus();

	if ( ret == CMD_RC_RUN )
	{
		ExecuteFile( Panel );
		return;
	}

	ret -= CMD_RC_OPEN_0;

	if ( ret < 0 || ret >= data.nodeList.count() )
	{
		return;
	}

	StartExecute( data.nodeList[ret].cmd, Panel->GetFS(), Panel->GetPath(), !data.nodeList[ret].terminal );
}

bool IsCommand_CD( const unicode_t* p )
{
#ifdef _WIN32
	return (p[0] == 'c' || p[0] == 'C') && (p[1] == 'd' || p[1] == 'D') && (!p[2] || p[2] == ' ');
#else
	return (p[0] == 'c' && p[1] == 'd' && (!p[2] || p[2] == ' '));
#endif
}

bool ApplyEnvVariable( const char* EnvVarName, std::vector<unicode_t>* Out )
{
	if ( !Out )
	{
		return false;
	}

	std::string Value = GetEnvVariable( EnvVarName );

	if ( Value.empty() )
	{
		return false;
	}

	*Out = utf8_to_unicode( Value.c_str() );
	return true;
}

// handle the "cd" command, convert its argument to a valid path, expand ~ and env variables
std::vector<unicode_t> ConvertCDArgToPath( const unicode_t* p )
{
	std::vector<unicode_t> Out;
	std::vector<unicode_t> Temp;

	while ( p && *p )
	{
		unicode_t Ch = 0;

		if ( *p == '~' )
		{
			if ( LookAhead( p, &Ch ) )
			{
				if ( (IsPathSeparator( Ch ) || Ch == 0) && ApplyEnvVariable( "HOME", &Temp ) )
				{
					// replace ~ with the HOME path
					Out.insert( Out.end(), Temp.begin(), Temp.end() );
					PopLastNull( &Out );
				}
			}
		}
		else if ( *p == '$' )
		{
			// skip `$`
			std::string EnvVarName = unicode_to_utf8( p + 1 );

			for ( auto i = EnvVarName.begin(); i != EnvVarName.end(); i++ )
			{
				if ( IsPathSeparator( *i ) )
				{
					*i = 0;
					break;
				}
			}

			if ( ApplyEnvVariable( EnvVarName.data(), &Temp ) )
			{
				// replace the var name with its value
				Out.insert( Out.end(), Temp.begin(), Temp.end() );
				PopLastNull( &Out );
				// skip var name
				p += strlen( EnvVarName.data() );
			}
		}
		else if ( IsPathSeparator( *p ) )
		{
			if ( !LastCharEquals( Out, '/' ) && !LastCharEquals( Out, '\\' ) )
			{
				Out.push_back( DIR_SPLITTER );
			}
		}
		else
		{
			Out.push_back( *p );
		}

		p++;
	}

	Out.push_back( 0 );

	// debug
	//	std::vector<char> U = unicode_to_utf8( Out.data() );
	//	const char* UTF = U.data();

	return Out;
}

bool FileExecutor::ProcessCommand_CD( const unicode_t* cmd, PanelWin* Panel )
{
	// make a mutable copy
	std::vector<unicode_t> copy = new_unicode_str( cmd );

	unicode_t* p = copy.data();

	//change dir
	m_History.Put( p );
	p += 2;

	SkipSpaces( p );

	std::vector<unicode_t> Path = ConvertCDArgToPath( p );

	if ( Path.empty() || !Path[0] )
	{
#if defined(_WIN32)
		StartExecute( cmd, Panel->GetFS(), Panel->GetPath() );
#else
		OpenHomeDir( Panel );
#endif
		return true;
	}

	p = Path.data();

	unicode_t* lastNoSpace = nullptr;

	for ( unicode_t* s = p; *s; s++ )
	{
		if ( *s != ' ' )
		{
			lastNoSpace = s;
		}
	}

	if ( lastNoSpace )
	{
		lastNoSpace[1] = 0;
	} //erase last spaces

	FSPath path = Panel->GetPath();

	ccollect<unicode_t, 0x100> pre;
	int sc = 0;

	while ( *p )
	{
		if ( sc )
		{
			if ( *p == sc )
			{
				sc = 0;  p++; continue;
			}
		}
		else if ( *p == '\'' || *p == '"' )
		{
			sc = *p;
			p++;
			continue;
		}

#ifndef _WIN32

		if ( *p == '\\' && !sc )
		{
			p++;
		}

#endif

		if ( !p )
		{
			break;
		}

		pre.append( *p );
		p++;
	}

	pre.append( 0 );
	p = pre.ptr();

	const std::vector<clPtr<FS>> checkFS =
	{
		Panel->GetFSPtr(),
		m_NCWin->GetOtherPanel( Panel )->GetFSPtr()
	};

	clPtr<FS> fs = ParzeURI( p, path, checkFS );

	if ( fs.IsNull() )
	{
		char buf[4096];
		FSString name = p;
		Lsnprintf( buf, sizeof( buf ), _LT( "can`t change directory to:%s\n" ), name.GetUtf8() );
		NCMessageBox( m_NCWin, "CD", buf, true );
	}
	else
	{
		Panel->LoadPath( fs, path, 0, 0, PanelWin::SET );
	}

	return true;
}

bool FileExecutor::ProcessCommand_CLS( const unicode_t* cmd )
{
	m_Terminal.TerminalReset( true );
	return true;
}

bool FileExecutor::ProcessBuiltInCommands( const unicode_t* cmd, PanelWin* Panel )
{
#if defined( _WIN32 ) || defined( __APPLE__ )
	bool CaseSensitive = false;
#else
	bool CaseSensitive = true;
#endif

	if ( IsEqual_Unicode_CStr( cmd, "cls", CaseSensitive ) )
	{
		ProcessCommand_CLS( cmd );
		return true;
	}
	
	if ( IsCommand_CD( cmd ) )
	{
		ProcessCommand_CD( cmd, Panel );
		return true;
	}

	return false;
}

bool FileExecutor::StartCommand( const std::vector<unicode_t>& CommandString, PanelWin* Panel, bool ForceNoTerminal, bool ReplaceSpecialChars )
{
	std::vector<unicode_t> Command = ReplaceSpecialChars ? MakeCommand( CommandString, Panel->GetCurrentFileName() ) : CommandString;

	const unicode_t* p = Command.data();

	SkipSpaces( p );

	//	printf( "StartCommand %s, %i\n", (const char*)p, (int)ForceNoTerminal );

	if ( !*p )
	{
		return false;
	}

	if ( *p )
	{
		m_History.ResetToLast();

		if ( !ProcessBuiltInCommands( p, Panel ) )
		{
			bool NoTerminal = (p[0] == '&' || ForceNoTerminal);

			if ( NoTerminal )
			{
				m_History.Put( p );

				if ( p[0] == '&' )
				{
					p++;
				}
			}

			FS* fs = Panel->GetFS();

			if ( fs && fs->Type() == FS::SYSTEM )
			{
				StartExecute( Command.data(), fs, Panel->GetPath(), NoTerminal );
			}
			else
			{
				NCMessageBox( m_NCWin, _LT( "Execute" ), _LT( "Can`t execute command in non system fs" ), true );
			}
		}
	}

	return true;
}

void FileExecutor::ApplyCommand( const std::vector<unicode_t>& cmd, PanelWin* Panel )
{
	clPtr<FSList> list = Panel->GetSelectedList();

	if ( !cmd.data() || !list.ptr() || list->Count() <= 0 )
	{
		return;
	}

	std::vector<FSNode*> nodes = list->GetArray();

	m_NCWin->SetMode( NCWin::TERMINAL );

	for ( auto i = nodes.begin(); i != nodes.end(); i++ )
	{
		FSNode* Node = *i;

		const unicode_t* Name = Node->GetUnicodeName();

		std::vector<unicode_t> Command = MakeCommand( cmd, Name );

		StartExecute( Command.data(), Panel->GetFS(), Panel->GetPath() );
	}
}

const clNCFileAssociation* FileExecutor::FindFileAssociation( const unicode_t* FileName ) const
{
	const auto& Assoc = g_Env.GetFileAssociations();

	for ( const auto& i : Assoc )
	{
		std::vector<unicode_t> Mask = i.GetMask();

		clMultimaskSplitter Splitter( Mask );

		if ( Splitter.CheckAndFetchAllMasks( FileName ) )
		{
			return &i;
		}
	}

	return nullptr;
}

bool FileExecutor::StartFileAssociation( PanelWin* panel, eFileAssociation Mode )
{
	const unicode_t* FileName = panel->GetCurrentFileName();

	const clNCFileAssociation* Assoc = FindFileAssociation( FileName );

	if ( !Assoc )
	{
		return false;
	}

	std::vector<unicode_t> Cmd = MakeCommand( Assoc->Get( Mode ), FileName );

	if ( Cmd.data() && *Cmd.data() )
	{
		StartExecute( Cmd.data(), panel->GetFS(), panel->GetPath(), !Assoc->GetHasTerminal() );
		return true;
	}

	return false;
}

void FileExecutor::ExecuteFileByEnter( PanelWin* Panel, bool Shift )
{
	FSNode* p = Panel->GetCurrent();

	bool cmdChecked = false;
	std::vector<unicode_t> cmd;
	bool terminal = true;
	const unicode_t* pAppName = 0;
	clPtr<FS> LocalFs = Panel->GetFSPtr();
	FSPath LocalPath = Panel->GetPath();

	FSString Uri;

	if ( !(LocalFs->Flags() & FS::HAVE_SEEK) )
	{
		// append file name to the path
		LocalPath.Push( CS_UTF8, p->name.GetUtf8() );

		// try to load virtual system file to local temp file
		if ( !LoadToTempFile( m_NCWin, &LocalFs, &LocalPath ) )
		{
			return;
		}

		// get full URI to the loaded temp local file
		Uri = LocalFs->Uri( LocalPath );
		
		// remove file name from the dir path
		LocalPath.Pop();
	}
	else
	{
		Uri = Panel->UriOfCurrent();
	}

	if ( Shift )
	{
		ExecuteDefaultApplication( Uri.GetUnicode() );
		return;
	}

	if ( StartFileAssociation( Panel, eFileAssociation_Execute ) )
	{
		return;
	}

	if ( g_WcmConfig.systemAskOpenExec )
	{
		cmd = GetOpenCommand( Uri.GetUnicode(), &terminal, &pAppName );
		cmdChecked = true;
	}

	if ( p->IsExe() )
	{
#ifndef _WIN32

		if ( g_WcmConfig.systemAskOpenExec && cmd.data() )
		{
			ButtonDataNode bListOpenExec[] = { { "&Open", CMD_OPEN_FILE }, { "&Execute", CMD_EXEC_FILE }, { "&Cancel", CMD_CANCEL }, { 0, 0 } };

			static unicode_t emptyStr[] = { 0 };

			if ( !pAppName )
			{
				pAppName = emptyStr;
			}

			int ret = NCMessageBox( m_NCWin, "Open",
				carray_cat<char>( "Executable file: ", p->name.GetUtf8(), "\ncan be opened by: ", unicode_to_utf8( pAppName ).data(), "\nExecute or Open?" ).data(),
				false, bListOpenExec );

			if ( ret == CMD_CANCEL )
			{
				return;
			}

			if ( ret == CMD_OPEN_FILE )
			{
				StartExecute( cmd.data(), LocalFs.ptr(), LocalPath, !terminal );
				return;
			}
		}

#endif
		ExecuteFile( Panel );
		return;
	}

	if ( !cmdChecked )
	{
		cmd = GetOpenCommand( Uri.GetUnicode(), &terminal, 0 );
	}

	if ( cmd.data() )
	{
		StartExecute( cmd.data(), LocalFs.ptr(), LocalPath, !terminal );
	}
}

void FileExecutor::ExecuteFile( PanelWin* panel )
{
	FSNode* p = panel->GetCurrent();

	if ( !p || p->IsDir() || !p->IsExe() )
	{
		return;
	}

	FS* fs = panel->GetFS();

	if ( !fs || fs->Type() != FS::SYSTEM )
	{
		NCMessageBox( m_NCWin, _LT( "Run" ), _LT( "Can`t execute file in not system fs" ), true );
		return;
	}

#ifdef _WIN32
	
	static unicode_t w[2] = { '"', 0 };
	StartExecute( carray_cat<unicode_t>( w, panel->UriOfCurrent().GetUnicode(), w ).data(), fs, panel->GetPath() );

#else

	const unicode_t*   fName = p->GetUnicodeName();
	int len = unicode_strlen( fName );
	std::vector<unicode_t> cmd( 2 + len + 1 );
	cmd[0] = '.';
	cmd[1] = '/';
	memcpy( cmd.data() + 2, fName, len * sizeof( unicode_t ) );
	cmd[2 + len] = 0;
	StartExecute( cmd.data(), fs, panel->GetPath() );

#endif
}

void FileExecutor::StartExecute( const unicode_t* cmd, FS* fs, FSPath& path, bool NoTerminal )
{
	SkipSpaces( cmd );

	if ( DoStartExecute( m_EditPref.Get(), cmd, fs, path ) )
	{
		m_History.Put( cmd );
		m_NCWin->SetMode( NCWin::TERMINAL );
	}

	ReturnToDefaultSysDir();
}

bool FileExecutor::DoStartExecute( const unicode_t* pref, const unicode_t* cmd, FS* fs, FSPath& path, bool NoTerminal )
{
#ifdef _WIN32

	if ( !m_Terminal.Execute( m_NCWin, TERMINAL_THREAD_ID, cmd, 0, fs->Uri( path ).GetUnicode() ) )
	{
		return false;
	}

#else

	static unicode_t empty[] = {0};
	static unicode_t newLine[] = { '\n', 0 };
	
	if ( !pref )
	{
		pref = empty;
	}

	if ( !*cmd )
	{
		return false;
	}

	m_Terminal.TerminalReset();

	if ( NoTerminal )
	{
		unsigned fg = 0xB;
		unsigned bg = 0;
		
		m_Terminal.TerminalPrint( newLine, fg, bg );
		m_Terminal.TerminalPrint( pref, fg, bg );
		m_Terminal.TerminalPrint( cmd, fg, bg );
		m_Terminal.TerminalPrint( newLine, fg, bg );

		char* dir = 0;

		if ( fs && fs->Type() == FS::SYSTEM )
		{
			dir = (char*) path.GetString( sys_charset_id );
		}

		FSString s = cmd;
		sys_char_t* SysCmd = (sys_char_t*) s.Get( sys_charset_id );

		pid_t pid = fork();
		if ( pid < 0 )
		{
			return false;
		}

		if ( pid )
		{
			waitpid( pid, 0, 0 );
		}
		else
		{
			if ( !fork() )
			{
				//printf("exec: %s\n", SysCmd);
				signal( SIGINT, SIG_DFL );
				static char shell[] = "/bin/sh";
				const char* params[] = { shell, "-c", SysCmd, NULL };

				if ( dir )
				{
					chdir( dir );
				}

				execv( shell, (char**) params );
				exit( 1 );
			}

			exit( 0 );
		}
	}
	else
	{
		unsigned fg_pref = 0xB;
		unsigned fg_cmd = 0xF;
		unsigned bg = 0;
		
		m_Terminal.TerminalPrint( newLine, fg_pref, bg );
		m_Terminal.TerminalPrint( pref, fg_pref, bg );
		m_Terminal.TerminalPrint( cmd, fg_cmd, bg );
		m_Terminal.TerminalPrint( newLine, fg_cmd, bg );

		int l = unicode_strlen( cmd );
		int i;

		if ( l >= 64 )
		{
			for ( i = 0; i < 64 - 1; i++ )
			{
				m_ExecSN[i] = cmd[i];
			}

			m_ExecSN[60] = '.';
			m_ExecSN[61] = '.';
			m_ExecSN[62] = '.';
			m_ExecSN[63] = 0;
		}
		else
		{
			for ( i = 0; i < l; i++ )
			{
				m_ExecSN[i] = cmd[i];
			}

			m_ExecSN[l] = 0;
		}

		m_Terminal.Execute( m_NCWin, TERMINAL_THREAD_ID, cmd, (sys_char_t*) path.GetString( sys_charset_id ) );
	}

#endif

	return true;
}

void FileExecutor::StopExecute()
{
#ifdef _WIN32

	if ( NCMessageBox( m_NCWin, _LT( "Stop" ), _LT( "Drop current console?" ), false, bListOkCancel ) == CMD_OK )
	{
		m_Terminal.DropConsole();
	}

#else

	if ( m_ExecId > 0 )
	{
		int ret = KillCmdDialog( m_NCWin, m_ExecSN );

		if ( m_ExecId > 0 )
		{
			if ( ret == CMD_KILL_9 )
			{
				kill( m_ExecId, SIGKILL );
			}
			else if ( ret == CMD_KILL )
			{
				kill( m_ExecId, SIGTERM );
			}
		}
	}

#endif
}

void FileExecutor::ThreadSignal( int id, int data )
{
	if ( id == TERMINAL_THREAD_ID )
	{
		m_ExecId = data;
	}
}

void FileExecutor::ThreadStopped( int id, void* data )
{
	if ( id == TERMINAL_THREAD_ID )
	{
		m_ExecId = -1;
		m_ExecSN[0] = 0;
	}
}
