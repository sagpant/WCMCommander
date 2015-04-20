/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * walcommander@linderdaum.com
 */

#pragma once

#include "fileassociations.h"

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
class PanelWin;

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

	/// Shows context menu at the given position for the selected file
	void ShowFileContextMenu( cpoint point, PanelWin* Panel );

	/// Runs the given command on the given panel
	bool StartCommand( const std::vector<unicode_t>& CommandString, PanelWin* Panel, bool ForceNoTerminal, bool ReplaceSpecialChars );

	/// Applies the given command to the selected files on the given panel
	void ApplyCommand( const std::vector<unicode_t>& cmd, PanelWin* Panel );

	/// Starts to execute current file on the given panel using file associations
	bool StartFileAssociation( PanelWin* panel, eFileAssociation Mode );

	/// Executes selected file when Enter key is pressed
	void ExecuteFileByEnter( PanelWin* Panel, bool Shift );

	/// Stops current run process in terminal
	void StopExecute();

	/// Called from main NCWin
	void ThreadSignal( int id, int data );

	/// Called from main NCWin
	void ThreadStopped( int id, void* data );

private:
	const clNCFileAssociation* FindFileAssociation( const unicode_t* FileName ) const;

	/// Starts to execute the given command in the given dir
	void StartExecute( const unicode_t* cmd, FS* fs, FSPath& path, bool NoTerminal = false );

	bool DoStartExecute( const unicode_t* pref, const unicode_t* cmd, FS* fs, FSPath& path, bool NoTerminal = false );

	/// Executes selected runnable file (.exe, .bat, etc.) from the given panel
	void ExecuteFile( PanelWin* panel );

	bool ProcessCommand_CD( const unicode_t* cmd, PanelWin* Panel );
	bool ProcessCommand_CLS( const unicode_t* cmd );
	bool ProcessBuiltInCommands( const unicode_t* cmd, PanelWin* Panel );
};
