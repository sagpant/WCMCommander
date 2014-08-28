#ifndef WCM_CONFIG_H
#define WCM_CONFIG_H

#include "ncdialogs.h"

class NCWin;

struct WcmConfig
{
	enum MapType { MT_BOOL, MT_INT, MT_STR };
	struct Node
	{
		MapType type;
		const char* section;
		const char* name;
		union
		{
			int* pInt;
			bool* pBool;
			carray<char>* pStr;
		} ptr;
		union
		{
			int defInt;
			bool defBool;
			const char* defStr;
		} def;
	};

	ccollect<Node, 64> mapList;
	void MapInt( const char* section, const char* name, int* pInt, int def );
	void MapBool( const char* section, const char* name, bool* pInt, bool def );
	void MapStr( const char* section, const char* name, carray<char>* pStr, const char* def = 0 );

	bool systemAskOpenExec;
	bool systemEscPanel;
	bool systemBackSpaceUpDir;
	carray<char> systemLang; //"+" - auto "-" -internal eng.

	bool showToolBar;
	bool showButtonBar;
//	bool whiteStyle;

	bool panelShowHiddenFiles;
	bool panelShowIcons;
	bool panelCaseSensitive;
	bool panelSelectFolders;
	int panelColorMode;
	int panelModeLeft;
	int panelModeRight;

	bool editSavePos;
	bool editAutoIdent;
	int editTabSize;
	int editColorMode;
	bool editShl;

	int terminalBackspaceKey;

	int viewColorMode;

	int windowX;
	int windowY;
	int windowWidth;
	int windowHeight;

	//fonts
	carray<char> panelFontUri;
	carray<char> viewerFontUri;
	carray<char> editorFontUri;
	carray<char> dialogFontUri;
	carray<char> terminalFontUri;
	carray<char> helpTextFontUri;
	carray<char> helpBoldFontUri;
	carray<char> helpHeadFontUri;

	carray<char> leftPanelPath;
	carray<char> rightPanelPath;

	void ImpCurrentFonts(); //взять используемые шрифты и записать их реквизиты в ...Uri

	WcmConfig();
	void Load();
	void Save( NCWin* nc );
};

extern WcmConfig wcmConfig;

void InitConfigPath();

bool DoPanelConfigDialog( NCDialogParent* parent );
bool DoEditConfigDialog( NCDialogParent* parent );
bool DoStyleConfigDialog( NCDialogParent* parent );
bool DoSystemConfigDialog( NCDialogParent* parent );
bool DoTerminalConfigDialog( NCDialogParent* parent );

bool LoadStringList( const char* section, ccollect< carray<char> >& list );
void SaveStringList( const char* section, ccollect< carray<char> >& list );

#endif
