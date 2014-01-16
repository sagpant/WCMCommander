#ifndef COLOR_STYLE_H
#define COLOR_STYLE_H

struct PanelItemColors {
	int bg;
	int text;
	int shadow;
};

struct PanelHeaderColors {
	int bg;
	int text;
};

struct PanelFooterColors {
	int bg;
	int curText;
	int selText;
	int operText;
	int text;
};

struct PanelColors {
	int bg;
	int modeLine;
	
	int border1;
	int border2;
	int border3;
	int border4;
	
	int hLine; 
	
	int v1; //разрисовка верт линий
	int v2;
	int v3;

	void (*itemF)(PanelItemColors *p,
		bool operPanel,
		bool active,
		bool selected,
		bool bad,
		bool hidden,
		bool dir,
		bool exe
	);

	void (*headF)(PanelHeaderColors *p, bool selectedPanel);
	
	PanelFooterColors foot;
};

void SetPanelColorStyle(int style);
extern PanelColors *panelColors;

struct EditorColors {
	int bg;
	int fg;
	int bgMarked;
	int fgMarked;
	int cursor;

	unsigned shlDEF;
	unsigned shlKEYWORD;
	unsigned shlCOMMENT;
	unsigned shlSTRING;
	unsigned shlPRE;
	unsigned shlNUM;
	unsigned shlOPER;
	unsigned shlATTN;
};

extern EditorColors *editorColors;
void SetEditorColorStyle(int style);

struct ViewerColors {
	int bg;
	int fg;
	
	int sym;
	int markFg;
	int markBg;
	
	int lnFg; //line number foreground (in hex mode)
	int hid;
	
	int loadBg;
	int loadFg;
};

extern ViewerColors *viewerColors;
void SetViewerColorStyle(int style);


#endif
