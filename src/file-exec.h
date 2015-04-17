/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * walcommander@linderdaum.com
 */

#pragma once

#ifdef _WIN32

#include "w32cons.h"
typedef W32Cons TerminalWin_t;

#else

#include "termwin.h"
typedef TerminalWin TerminalWin_t;

#endif


class NCWin;
class StringWin;
class NCHistory;
class FS;
class FSPath;


/// Contains logic for running and opening files
class FileExecutor
{
private:

	NCWin* m_NCWin;
	StringWin& _editPref;
	NCHistory& _history;
	TerminalWin_t& _terminal;

	int _execId;
	unicode_t _execSN[64];

	FileExecutor( const FileExecutor& ) = delete;
	FileExecutor& operator=(const FileExecutor&) = delete;

public:
	FileExecutor( NCWin* NCWin, StringWin& editPref, NCHistory& history, TerminalWin_t& terminal );
	virtual ~FileExecutor() {}

	/// Start to execute the given command in the given dir
	void StartExecute( const unicode_t* cmd, FS* fs, FSPath& path, bool NoTerminal = false );

	/// Stops current run process in terminal
	void StopExecute();

	/// Called from main NCWin
	void ThreadSignal( int id, int data );

	/// Called from main NCWin
	void ThreadStopped( int id, void* data );

private:
	bool StartExecute( const unicode_t* pref, const unicode_t* cmd, FS* fs, FSPath& path, bool NoTerminal = false );
};
