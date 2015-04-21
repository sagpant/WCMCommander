/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * wcm@linderdaum.com
 */

#pragma once

#include "ncdialogs.h"

#include <string>

class NCWin;


class IniHash
{
private:
	cstrhash< cstrhash< std::string > > hash;
	std::string* Find( const char* section, const char* var );
	std::string* Create( const char* section, const char* var );
	void Delete( const char* section, const char* var );

public:
	IniHash() { }
	~IniHash() { }

	void SetStrValue( const char* section, const char* var, const char* value );
	void SetIntValue( const char* section, const char* var, int value );
	void SetBoolValue( const char* section, const char* var, bool value );
	const char* GetStrValue( const char* section, const char* var, const char* def );
	int GetIntValue( const char* section, const char* var, int def );
	bool GetBoolValue( const char* section, const char* var, bool def );
	
	void Clear()
	{
		hash.clear();
	}

	int Size() const
	{
		return hash.count();
	}

	std::vector<const char*> Keys()
	{
		 return hash.keys();
	}

	cstrhash<std::string>* Exist( const char* key)
	{
		 return hash.exist( key );
	}
};

void IniHashLoad( IniHash& iniHash, const char* sectName );
void IniHashSave( IniHash& iniHash, const char* sectName );


enum ePanelSpacesMode
{
	ePanelSpacesMode_None = 0,
	ePanelSpacesMode_All = 1,
	ePanelSpacesMode_Trailing = 2
};

class clWcmConfig
{
private:
	enum eMapType { MT_BOOL, MT_INT, MT_STR };
	/// no need to make this polymorphic type
	struct sNode
	{
		sNode()
			: m_Type( MT_INT )
			, m_Section( NULL )
			, m_Name( NULL )
			, m_Current()
			, m_Default()
		{}
		sNode( eMapType Type, const char* Section, const char* Name )
			: m_Type( Type )
			, m_Section( Section )
			, m_Name( Name )
			, m_Current()
			, m_Default()
		{}
		eMapType    m_Type;
		const char* m_Section;
		const char* m_Name;
		/// holds a pointer to the currently valid value
		union
		{
			int* m_Int;
			bool* m_Bool;
			std::string* m_Str;
		} m_Current;

		int GetDefaultInt() const { return m_Default.m_Int; }
		bool GetDefaultBool() const { return m_Default.m_Bool; }
		const char* GetDefaultStr() const { return m_Default.m_Str; }

		int GetCurrentInt() const { return m_Current.m_Int ? *m_Current.m_Int : 0; }
		bool GetCurrentBool() const { return m_Current.m_Bool ? *m_Current.m_Bool : false; }
		const char* GetCurrentStr() const { return m_Current.m_Str ? m_Current.m_Str->data() : NULL; }

		static sNode CreateIntNode( const char* Section, const char* Name, int* pInt, int DefaultValue )
		{
			sNode N( MT_INT, Section, Name );
			N.m_Current.m_Int = pInt;
			N.m_Default.m_Int = DefaultValue;
			return N;
		}
		static sNode CreateBoolNode( const char* Section, const char* Name, bool* pBool, bool DefaultValue )
		{
			sNode N( MT_BOOL, Section, Name );
			N.m_Current.m_Bool = pBool;
			N.m_Default.m_Bool = DefaultValue;
			return N;
		}
		static sNode CreateStrNode( const char* Section, const char* Name, std::string* pStr, const char* DefaultValue )
		{
			sNode N( MT_STR, Section, Name );
			N.m_Current.m_Str = pStr;
			N.m_Default.m_Str = DefaultValue;
			return N;
		}

	private:
		/// default value
		union
		{
			int m_Int;
			bool m_Bool;
			const char* m_Str;
		} m_Default;
	};

public:

	#pragma region System settings
	bool systemAskOpenExec;
	bool systemEscPanel;
	bool systemEscCommandLine;
	bool systemBackSpaceUpDir;
	bool systemAutoComplete;
	bool systemAutoSaveSetup;
	bool systemShowHostName;
	bool systemStorePasswords;
	std::string systemLang; //"+" - auto "-" -internal eng.
	#pragma endregion

	#pragma region Panel settings
	bool panelShowHiddenFiles;
	bool panelCaseSensitive;
	bool panelSelectFolders;
	bool panelShowDotsInRoot;
	bool panelShowFolderIcons;
	bool panelShowExecutableIcons;
	bool panelShowLinkIcons;
	bool panelShowScrollbar;
	ePanelSpacesMode panelShowSpacesMode;
	int panelModeLeft;
	int panelModeRight;
	#pragma endregion

	#pragma region Editor settings
	bool editSavePos;
	bool editAutoIdent;
	int editTabSize;
	bool editShl;
	bool editClearHistoryAfterSaving;
	#pragma endregion

	#pragma region Terminal settings
	int terminalBackspaceKey;
	#pragma endregion

	#pragma region Style settings
	bool styleShow3DUI;
	std::string styleColorTheme;
	bool styleShowToolBar;
	bool styleShowButtonBar;
	bool styleShowButtonBarIcons;
	bool styleShowMenuBar;
	#pragma endregion

	#pragma region Window position and size to be restored on the next startup
	int windowX;
	int windowY;
	int windowWidth;
	int windowHeight;
	#pragma endregion

	#pragma region Fonts
	std::string panelFontUri;
	std::string viewerFontUri;
	std::string editorFontUri;
	std::string dialogFontUri;
	std::string terminalFontUri;
	std::string helpTextFontUri;
	std::string helpBoldFontUri;
	std::string helpHeadFontUri;

	/// store properties of the currently active fonts in ...Uri fields
	void ImpCurrentFonts();
	#pragma endregion

	#pragma region Paths of the panels to be restored on the next startup
	std::string leftPanelPath;
	std::string rightPanelPath;
	#pragma endregion

	clWcmConfig();
	void Load( NCWin* nc, const std::string& StartupDir );
	void Save( NCWin* nc );

private:
	void MapInt( const char* Section, const char* Name, int* pInt, int DefaultValue );
	void MapBool( const char* Section, const char* Name, bool* pInt, bool DefaultValue );
	void MapStr( const char* Section, const char* Name, std::string* pStr, const char* DefaultValue = NULL );

private:
	std::vector<sNode> m_MapList;
};

void InitConfigPath();

bool DoPanelConfigDialog( NCDialogParent* parent );
bool DoEditConfigDialog( NCDialogParent* parent );
bool DoStyleConfigDialog( NCDialogParent* parent );
bool DoSystemConfigDialog( NCDialogParent* parent );
bool DoTerminalConfigDialog( NCDialogParent* parent );

bool LoadStringList( const char* section, std::vector< std::string >& list );
void SaveStringList( const char* section, std::vector< std::string >& list );
