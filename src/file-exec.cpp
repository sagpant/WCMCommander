/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * walcommander@linderdaum.com
 */

#include "file-exec.h"
#include "ncwin.h"
#include "ltext.h"
#include "string-util.h"
#include "panel.h"

#ifndef _WIN32
#  include <signal.h>
#  include <sys/wait.h>
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


FileExecutor::FileExecutor( NCWin* NCWin, StringWin& editPref, NCHistory& history, TerminalWin_t& terminal )
	: m_NCWin( NCWin )
	, _editPref( editPref )
	, _history( history )
	, _terminal( terminal )
	, _execId( -1 )
{
	_execSN[0] = 0;
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

	if ( StartExecute( _editPref.Get(), cmd, fs, path ) )
	{
		_history.Put( cmd );
		m_NCWin->SetMode( NCWin::TERMINAL );
	}

	ReturnToDefaultSysDir();
}

bool FileExecutor::StartExecute( const unicode_t* pref, const unicode_t* cmd, FS* fs, FSPath& path, bool NoTerminal )
{
#ifdef _WIN32

	if ( !_terminal.Execute( m_NCWin, TERMINAL_THREAD_ID, cmd, 0, fs->Uri( path ).GetUnicode() ) )
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

	_terminal.TerminalReset();

	if ( NoTerminal )
	{
		unsigned fg = 0xB;
		unsigned bg = 0;
		
		_terminal.TerminalPrint( newLine, fg, bg );
		_terminal.TerminalPrint( pref, fg, bg );
		_terminal.TerminalPrint( cmd, fg, bg );
		_terminal.TerminalPrint( newLine, fg, bg );

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
		
		_terminal.TerminalPrint( newLine, fg_pref, bg );
		_terminal.TerminalPrint( pref, fg_pref, bg );
		_terminal.TerminalPrint( cmd, fg_cmd, bg );
		_terminal.TerminalPrint( newLine, fg_cmd, bg );

		int l = unicode_strlen( cmd );
		int i;

		if ( l >= 64 )
		{
			for ( i = 0; i < 64 - 1; i++ )
			{
				_execSN[i] = cmd[i];
			}

			_execSN[60] = '.';
			_execSN[61] = '.';
			_execSN[62] = '.';
			_execSN[63] = 0;
		}
		else
		{
			for ( i = 0; i < l; i++ )
			{
				_execSN[i] = cmd[i];
			}

			_execSN[l] = 0;
		}

		_terminal.Execute( m_NCWin, TERMINAL_THREAD_ID, cmd, (sys_char_t*) path.GetString( sys_charset_id ) );
	}

#endif

	return true;
}

void FileExecutor::StopExecute()
{
#ifdef _WIN32

	if ( NCMessageBox( m_NCWin, _LT( "Stop" ), _LT( "Drop current console?" ), false, bListOkCancel ) == CMD_OK )
	{
		_terminal.DropConsole();
	}

#else

	if ( _execId > 0 )
	{
		int ret = KillCmdDialog( m_NCWin, _execSN );

		if ( _execId > 0 )
		{
			if ( ret == CMD_KILL_9 )
			{
				kill( _execId, SIGKILL );
			}
			else if ( ret == CMD_KILL )
			{
				kill( _execId, SIGTERM );
			}
		}
	}

#endif
}

void FileExecutor::ThreadSignal( int id, int data )
{
	if ( id == TERMINAL_THREAD_ID )
	{
		_execId = data;
	}
}

void FileExecutor::ThreadStopped( int id, void* data )
{
	if ( id == TERMINAL_THREAD_ID )
	{
		_execId = -1;
		_execSN[0] = 0;
	}
}
