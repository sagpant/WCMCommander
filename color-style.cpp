#include "color-style.h"


static 	void DefPanelItemColor(PanelItemColors *p, bool oper, bool active, bool selected, bool bad, bool hidden, bool dir, bool exe )
{
	p->bg =  active ? (oper ? 0xFF : 0xB0B000) : 0x800000;
	p->shadow = 0;

	if (selected)	p->text = 0x00FFFF;
	else
	if (bad)	p->text = 0xA0;
	else 	
	if (hidden)	p->text = active ? 0 : 0xFF8080;
	else 
	if (dir)	p->text = 0xFFFFFF;
	else 
	if (exe)	p->text = 0x80FF80;
	else 
		p->text = active ? 0x1 : 0xFFFF80;
}

static void DefPanelHeadColor(PanelHeaderColors *p, bool selectedPanel)
{
	p->bg = 0xA00000;
	p->text = selectedPanel ? 0xFFFFFF : 0xFFD0D0;
}


PanelColors defPanelColors =  
{
	0x800000, //bg
	0xFFFFFF, //int modeLine;
	
	0x000000, //int border1;
	0xFFFFFF, //int border2;
	0xFFFFFF, //int border3;
	0x000000, //int border4;
	
	0xFFFFFF, //int hLine; 
	
	0xFFFFFF, //int v1; //разрисовка верт линий
	0x707070, //int v2;
	0x700000, //int v3;

	DefPanelItemColor,
	DefPanelHeadColor,
	
	//PanelFootColor foot;
	{
		0xA00000, //int bg;
		0xFFFF00, //int curText;
		0x00FFFF, //int selText;
		0xFFFF00, //int operText;
		0xFFFFFF //int text;
	}
};



static 	void BlackPanelItemColor(PanelItemColors *p, bool oper, bool active, bool selected, bool bad, bool hidden, bool dir, bool exe )
{
	p->bg = active ? (oper ? 0xFF : 0xB0B000) : 0;
	p->shadow = 0;

	if (selected)	p->text = 0x00FFFF;
	else
	if (bad)	p->text = 0xA0;
	else 	
	if (hidden)	p->text = active ? 0 : 0xFF8080;
	else 
	if (dir)	p->text = 0xFFFFFF;
	else 
	if (exe)	p->text = 0x80FF80;
	else 
		p->text = active ? 0x1 : 0xA0A0A0;//FFFF80;
}

static void BlackPanelHeadColor(PanelHeaderColors *p, bool selectedPanel)
{
	p->bg = 0;
	p->text = selectedPanel ? 0xFFFFFF : 0xFFD0D0;
}


PanelColors blackPanelColors = 
{
	0, //bg
	0xFFFFFF, //int modeLine;
	
	0x000000, //int border1;
	0xFFFFFF, //int border2;
	0xFFFFFF, //int border3;
	0x000000, //int border4;
	
	0xFFFFFF, //int hLine; 
	
	0xFFFFFF, //int v1; //разрисовка верт линий
	0x707070, //int v2;
	0x700000, //int v3;

	BlackPanelItemColor,
	BlackPanelHeadColor,
	
	//PanelFootColor foot;
	{
		0, //int bg;
		0xFFFF00, //int curText;
		0x00FFFF, //int selText;
		0xFFFF00, //int operText;
		0xFFFFFF //int text;
	}
};


static 	void WhitePanelItemColor(PanelItemColors *p, bool oper, bool active, bool selected, bool bad, bool hidden, bool dir, bool exe )
{
	p->bg = active ? (oper ? 0xFF : 0xB8C9CC) : 0xFFFFFF;
	p->shadow = 0x707070;

	if (selected)	p->text = 0x0000FF; //0x00E2E2;
	else
	if (bad)	p->text = 0xA0;
	else 	
	if (hidden)	p->text = active ? 0 : 0xA08080;
	else 
	if (dir)	p->text = 0;
	else 
	if (exe)	p->text = 0x408010;
	else 
		p->text = active ? 0 : 0;
}

static void WhitePanelHeadColor(PanelHeaderColors *p, bool selectedPanel)
{
	p->bg = 0xD8E9EC;
	p->text = selectedPanel ? 0 : 0x808080;
}


PanelColors whitePanelColors = {
	0xFFFFFF, //bg
	0x0, //int modeLine;
	
	0x808080, //int border1;
	0xD0D0D0, //int border2;
	0xD8E9EC, //int border3;
	0x808080, //int border4;
	
	0x707070, //int hLine; 
	
	0xC0C0C0, //int v1; //разрисовка верт линий
	0xD8E9EC, //int v2;
	0x808080, //int v3;

	WhitePanelItemColor,
	WhitePanelHeadColor,
	
	//PanelFootColor foot;
	{
		0xD8E9EC, //int bg;
		0x606000, //int curText;
		0x0000FF, //int selText;
		0x808000, //int operText;
		0 //int text;
	}
};


PanelColors *panelColors = &defPanelColors;

void SetPanelColorStyle(int style)
{
	switch (style){
	case 1: panelColors  = &blackPanelColors; break;
	case 2: panelColors  = &whitePanelColors; break;
	default: panelColors  = &defPanelColors;
	}
}

EditorColors defEditorColors = 
{
	0x800000,
	0xFFFFFF,
	0xFFFFFF,
	0,
	0xFFFF00,
	0xFFFFFF,// DEF
	0x00FFFF,// KEYWORD
	0xFF8080,// COMMENT
	0xFFFF00,// STRING
	0x00FF00,// PRE
	0xFFFF80,// NUM
	0x20FFFF,// OPER
	0x0000FF // ATTN
};

EditorColors blackEditorColors = 
{
	0x0,
	0xFFFFFF,
	0xFFFFFF,
	0,
	0xFFFF00,
	0xE0E0E0, //FFFFFF,// DEF
	0x6080F0, //0x00FFFF,// KEYWORD
	0xA08080,// COMMENT
	0xFFFF00,// STRING
	0x00FF00,// PRE
	0xFFFF80,// NUM
	0x20FFFF,// OPER
	0x0000FF // ATTN
};

EditorColors whiteEditorColors = 
{
	0xFFFFFF,
	0,
	0,
	0xFFFFFF,
	0x8000,
	0x0,	// DEF
	0xE00000, //0xFF0000,// KEYWORD
	0x008000, //0x00D000,// COMMENT
	0xA0A000, //0xB0B000,// STRING
	0x800080, //0xFF00FF,// PRE
	0x808000, //0x808000,// NUM
	0x008080, //0x008080,// OPER
	0x0000FF // ATTN
};



EditorColors *editorColors = &defEditorColors;;

void SetEditorColorStyle(int style)
{
	switch (style){
	case 1: editorColors  = &blackEditorColors; break;
	case 2: editorColors  = &whiteEditorColors; break;
	default: editorColors  = &defEditorColors;
	}
}

static ViewerColors defViewerColors = {
	0x800000, //bg
	0xFFFFFF, //fg
	0xFF0000, //sym;
	0x0,	  //markFg;
	0xFFFFFF, //markBg;
	0x00FFFF, // lnFg; //line number foreground (in hex mode)
	0xB00000,
	0xD00000, //loadBg
	0xFF	  //loadFb
};

static ViewerColors blackViewerColors = {
	0,
	0xA0A0A0,
	0x00FFFF,
	0x0,	  //markFg;
	0xFFFFFF, //markBg;
	0xFF0000,
	0x202020,
	0xD00000, //loadBg
	0xFF
};

static ViewerColors whiteViewerColors = {
	0xFFFFFF,
	0x0,
	0x008080,
	0xFFFFFF,  //markFg;
	0x0, //markBg;
	0xFF,
	0xE0E0E0,
	0xD00000, //loadBg
	0xFF
};

ViewerColors *viewerColors = &defViewerColors;

void SetViewerColorStyle(int style)
{
	switch (style){
	case 1: viewerColors  = &blackViewerColors; break;
	case 2: viewerColors  = &whiteViewerColors; break;
	default: viewerColors  = &defViewerColors;
	}
}
