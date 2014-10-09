/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#pragma once

#include "swl.h"
#include "panel.h"
#ifdef _WIN32
#	include "w32cons.h"
#else
#	include "termwin.h"
#endif

#include "ncview.h"
#include "ncedit.h"
#include "nchistory.h"
#include "ncdialogs.h"
#include "ext-app.h"
#include "toolbar.h"
#include "fileassociations.h"

using namespace wal;

class StringWin: public Win
{
	std::vector<unicode_t> text;
	cpoint textSize;
public:
	StringWin( Win* parent );
	virtual void Paint( wal::GC& gc, const crect& paintRect );
	const unicode_t* Get() { return text.data(); }
	void Set( const unicode_t* txt );
	void Set( const std::vector<unicode_t>& txt );
	virtual void OnChangeStyles();
	virtual int UiGetClassId();
	virtual  ~StringWin();
};


struct ButtonWinData
{
	const char* txt;
	int commandId;
};

class ButtonWin: public Win
{
	Layout _lo;
	clPtr<Button> _buttons[10];
	crect _rects[10];
	cpoint _nSizes[10];

	ButtonWinData* lastData;
public:
	ButtonWin( Win* parent );
	void Set( ButtonWinData* list );
	ButtonWinData* GetLastData() { return lastData; }
	virtual int UiGetClassId();
	virtual void Paint( wal::GC& gc, const crect& paintRect );
	virtual void OnChangeStyles();
	virtual ~ButtonWin();
};

#define CI_EDITORHEADWIN 100


template <int N> class UFStr
{
	unicode_t data[N];
public:
	UFStr() { data[0] = 0;}
	void Set( const unicode_t* s ) { int i; for ( i = 0; i < N - 1 && *s; i++, s++ ) { data[i] = *s; } data[i] = 0; }
	UFStr( const unicode_t* s ) { Set( s ); }
	bool Eq( const unicode_t* s ) { int i = 0; while ( (i < N - 1) && data[i] && (data[i] == *s) ) { i++; s++; }; return (data[i] == 0 && *s == 0) || i >= N; }
	unicode_t* Str() { return data; }
};

class EditorHeadWin: public Win
{
	EditWin* _edit;
	int chW;
	int chH;

	UFStr<32> prefixString;
	crect prefixRect;
	int prefixWidth;

	UFStr<0x100> nameString;
	crect nameRect;

	UFStr<32> symString;
	crect symRect;

	UFStr<32> csString;
	crect csRect;

	UFStr<32> posString;
	crect posRect;

	bool UpdateName();
	bool UpdateSym();
	void DrawSym( wal::GC& gc );
	bool UpdateCS();
	void DrawCS( wal::GC& gc );
	bool UpdatePos();
	void DrawPos( wal::GC& gc );

	void CheckSize();
public:
	EditorHeadWin( Win* parent, EditWin* pEdit );
	virtual void Paint( wal::GC& gc, const crect& paintRect );
	virtual ~EditorHeadWin();
	virtual bool Broadcast( int id, int subId, Win* win, void* data );
	virtual void EventSize( cevent_size* pEvent );
	virtual bool EventMouse( cevent_mouse* pEvent );
	virtual int UiGetClassId();
	virtual void OnChangeStyles();
};


class ViewerHeadWin: public Win
{
	ViewWin* _view;
	int chW;
	int chH;

	UFStr<32> prefixString;
	crect prefixRect;
	int prefixWidth;

	UFStr<0x100> nameString;
	crect nameRect;

	UFStr<32> colString;
	crect colRect;

	UFStr<32> csString;
	crect csRect;

	UFStr<32> percentString;
	crect percentRect;

	bool UpdateName();

	bool UpdateCol();
	void DrawCol( wal::GC& gc );

	bool UpdateCS();
	void DrawCS( wal::GC& gc );

	bool UpdatePercent();
	void DrawPercent( wal::GC& gc );

	void CheckSize();
public:
	ViewerHeadWin( Win* parent, ViewWin* pView );
	virtual void Paint( wal::GC& gc, const crect& paintRect );
	virtual ~ViewerHeadWin();
	virtual bool Broadcast( int id, int subId, Win* win, void* data );
	virtual void EventSize( cevent_size* pEvent );
	virtual bool EventMouse( cevent_mouse* pEvent );
	virtual int UiGetClassId();
	virtual void OnChangeStyles();
};

class NCCommandLine: public EditLine
{
public:
	NCCommandLine( int nId, Win* parent, const crect* rect, const unicode_t* txt, int chars, bool frame );
	int UiGetClassId() override;
	bool EventKey( cevent_key* pEvent ) override;
};

class NCAutocompleteList: public TextList
{
public:
	NCAutocompleteList( WTYPE t, unsigned hints, int nId, Win* _parent, SelectType st, BorderType bt, crect* rect );
	int UiGetClassId() override;
	virtual bool EventKey( cevent_key* pEvent ) override;
	virtual bool EventMouse( cevent_mouse* pEvent ) override;
};

enum eBackgroundActivity
{
	eBackgroundActivity_None,
	eBackgroundActivity_Viewer,
	eBackgroundActivity_Editor,
};

class NCWin: public NCDialogParent
{
	friend class PanelWin;
	enum MODE { PANEL, TERMINAL, VIEW, EDIT };
private:
	Layout   _lo,
	         _lpanel,
	         _ledit;
	ButtonWin _buttonWin;
	PanelWin _leftPanel,
	         _rightPanel;

	NCAutocompleteList m_AutoCompleteList;
	std::vector<unicode_t> m_PrevAutoCurrentCommand;

	void UpdateAutoComplete( const std::vector<unicode_t>& CurrentCommand );

	NCCommandLine m_Edit;
#ifdef _WIN32
	W32Cons
#else
	TerminalWin
#endif
	_terminal;

	eBackgroundActivity m_BackgroundActivity;

	void UpdateActivityNotification();
	void SetBackgroundActivity( eBackgroundActivity m_BackgroundActivity );
	void SwitchToBackgroundActivity();

	StringWin _activityNotification;

	StringWin _editPref;
	PanelWin* _panel;

	MenuData _mdLeft, _mdRight, _mdFiles, _mdOptions, _mdCommands;
	MenuData _mdLeftSort, _mdRightSort;
	MenuBar _menu;

	ToolBar _toolBar;

	ViewWin _viewer;
	ViewerHeadWin _vhWin;

	EditWin _editor;
	EditorHeadWin _ehWin;

	bool _panelVisible;
	MODE _mode;

	NCHistory _history;

	/// currently active file associations
	std::vector<clNCFileAssociation> m_FileAssociations;

	LPanelSelectionType _shiftSelectType;

	void SetMode( MODE m );
	void ShowPanels( bool show )
	{
		if ( _panelVisible == show ) { return; }

		_panelVisible = show;

		if ( _mode == PANEL )
		{
			if ( _panelVisible )
			{ _leftPanel.Show(); _rightPanel.Show(); _terminal.Hide();}
			else
			{ _leftPanel.Hide(); _rightPanel.Hide(); _terminal.Show();}

			RecalcLayouts();
		}
	}

	void PanelEnter();
	void PanelCtrlPgDown();

	void ApplyCommandToList( const std::vector<unicode_t>& cmd, clPtr<FSList> list, PanelWin* Panel );
	void StartExecute( const unicode_t* cmd, FS* fs, FSPath& path );
	void ReturnToDefaultSysDir(); // !!!

	void Home( PanelWin* p );

	void ApplyCommand();
	void CreateDirectory();
	void View( bool Secondary );
	void ViewExit();
	void ViewCharsetTable();
	void ViewSearch( bool next );

	void Edit( bool enterFileName, bool Secondary );
	bool EditSave( bool saveAs );
	void EditExit();
	void EditNextCharset();
	void EditSearch( bool next );
	void EditReplace();
	void EditCharsetTable();

	void QuitQuestion();
	void Delete();
	void Copy( bool shift );
	void Move( bool shift );
	void Mark( bool enable );
	void CtrlEnter();
	void CtrlF();
	void HistoryDialog();
	void SelectDrive( PanelWin* p, PanelWin* OtherPanel );
	void SaveSetupDialog();
	void SaveSetup();
	void Search();
	void Tab( bool forceShellTab );
	void PanelEqual();
	void Shortcuts();
	void FileAssociations();
	void OnOffShl();

	void SetToolbarPanel();
	void SetToolbarEdit();
	void SetToolbarView();

	void PastePanelPath( PanelWin* p, bool AddTrailingSpace );
	void PastePanelCurrentFileURI( PanelWin* p, bool AddTrailingSpace );

	/// paste file or folder name to the command line, add quotes if the name contains spaces
	void PasteFileNameToCommandLine( const unicode_t* Path, bool AddTrailingSpace, bool AddPathSeparator );

	void CheckKM( bool ctrl, bool alt, bool shift, bool pressed, int ks );

	void ExecuteFile();

	const unicode_t* GetCurrentFileName() const;
	PanelWin* GetOtherPanel()
	{
		return _panel == &_leftPanel ? &_rightPanel : &_leftPanel;
	}

	PanelWin* GetOtherPanel(PanelWin* panel)
	{
		return panel == &_leftPanel ? &_rightPanel : &_leftPanel;
	}

#ifndef _WIN32
	void ExecNoTerminalProcess( const unicode_t* p );
#endif

	void RightButtonPressed( cpoint point ); //вызывается из панели, усли попало на имя файла/каталого

	clPtr<FSList> GetPanelList();

	int _execId;
	unicode_t _execSN[64];

	std::vector<unicode_t> FetchAndClearCommandLine();
	bool StartCommand( const std::vector<unicode_t>& cmd, bool ForceNoTerminal );

public:
	NCWin();
	bool OnKeyDown( Win* w, cevent_key* pEvent, bool pressed );
	virtual bool EventChildKey( Win* child, cevent_key* pEvent );
	virtual bool EventKey( cevent_key* pEvent );
	virtual void EventSize( cevent_size* pEvent );
	virtual void ThreadStopped( int id, void* data );
	virtual void ThreadSignal( int id, int data );
	virtual bool Command( int id, int subId, Win* win, void* data );
	virtual bool EventClose();

	int SendConfigChanged() { return SendBroadcast( ID_CHANGED_CONFIG_BROADCAST, 0, this, 0, 10 ); };

	virtual ~NCWin();

	PanelWin* GetLeftPanel() { return &_leftPanel; }
	PanelWin* GetRightPanel() { return &_rightPanel; }

	void HideAutoComplete();
	void NotifyAutoComplete();
	void NotifyAutoCompleteChange();
	void NotifyCurrentPathInfo();

	NCHistory* GetHistory() { return &_history; }
	const std::vector<clNCFileAssociation>& GetFileAssociations() const { return m_FileAssociations; }
	void SetFileAssociations( const std::vector<clNCFileAssociation>& Assoc ) { m_FileAssociations = Assoc; }

	const clNCFileAssociation* FindFileAssociation( const unicode_t* FileName ) const;
	bool StartFileAssociation( const unicode_t* FileName, eFileAssociation Mode );
};
