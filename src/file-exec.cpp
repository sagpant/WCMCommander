/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * walcommander@linderdaum.com
 */

#include "file-exec.h"
#include "ncwin.h"
#include "ltext.h"

#ifndef _WIN32
#  include <signal.h>
#  include <sys/wait.h>
#endif


FileExecutor::FileExecutor( NCWin* NCWin, TerminalWin_t& terminal )
	: m_NCWin( NCWin )
	, _terminal( terminal )
	, _execId( -1 )
{
	_execSN[0] = 0;
}

bool FileExecutor::StartExecute( const unicode_t* pref, const unicode_t* cmd, FS* fs, FSPath& path )
{
#ifdef _WIN32

	if ( !_terminal.Execute( m_NCWin, 1, cmd, 0, fs->Uri( path ).GetUnicode() ) )
	{
		return false;
	}

#else

	static unicode_t empty[] = { 0 };
	if ( !pref )
	{
		pref = empty;
	}

	_terminal.TerminalReset();
	unsigned fg_pref = 0xB;
	unsigned fg_cmd = 0xF;
	unsigned bg = 0;
	static unicode_t newLine[] = { '\n', 0 };
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

	_terminal.Execute( m_NCWin, 1, cmd, (sys_char_t*) path.GetString( sys_charset_id ) );

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
	if ( id == 1 )
	{
		_execId = data;
	}
}

void FileExecutor::ThreadStopped( int id, void* data )
{
	_execId = -1;
	_execSN[0] = 0;
}
