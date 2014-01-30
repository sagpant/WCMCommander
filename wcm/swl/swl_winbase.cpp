/*
	Copyright (c) by Valery Goryachev (Wal)
*/


#include "swl.h"
#include <string.h>

namespace wal {

unicode_t ABCString[]={'A','B','C',0};
int ABCStringLen = 3;

static cptr<cfont> pSysFont;

static cfont *guiFont = 0;

static unsigned winColors[]={
	0xD8E9EC,	//background
	0,		//foreground
	0,		//text color
	0x99080C,	//grey text color
	0x00A000	//focus mark color
};

static unsigned colorEditTextBg = 0xFFFFFF;
static unsigned colorEditText = 0;
static unsigned colorEditSTextBg = 0x800000; 
static unsigned colorEditSText = 0xFFFFFF;
static unsigned colorEditDisabledTextBg = 0x808080;
static unsigned colorEditDisabledText = 0;

static unsigned colorMBoxBg = 0xD8E9EC;
static unsigned colorMBoxBorder = 0xC0C0C0;
static unsigned colorMBoxText = 0;
static unsigned colorMBoxSelectBg = 0xFFA0A0;
static unsigned colorMBoxSelectText = 0;
static unsigned colorMBoxSelectFrame = 0;

static unsigned colorMPBg = 0xD8E9EC;
static unsigned colorMPLeft=0xE0E0E0;
static unsigned colorMPBorder=0;
static unsigned colorMPLine=0xC0C0C0;
static unsigned colorMPText=0;
static unsigned colorMPSText=0;
static unsigned colorMPSBg=0xFFA0A0;
static unsigned colorMPPointer=0;


#ifdef _WIN32
void BaseInit()
{
	static bool initialized = false;
	if (initialized) return;
	initialized =true;

	pSysFont = new cfont((HFONT)GetStockObject(DEFAULT_GUI_FONT));
	guiFont = pSysFont.ptr();

	winColors[IC_BG] = ::GetSysColor(COLOR_BTNFACE);
	winColors[IC_FG] = winColors[IC_TEXT] = ::GetSysColor(COLOR_BTNTEXT);
	winColors[IC_GRAY_TEXT] = ::GetSysColor(COLOR_GRAYTEXT);
}
#else 

void BaseInit()
{
	dbg_printf("BaseInit\n");
	static bool initialized = false;
	if (initialized) return;
	initialized =true;
	GC gc((Win*)0);
	pSysFont = new cfont(gc,"fixed",12,cfont::Normal); // "helvetica",9);
	guiFont = pSysFont.ptr();
}

#endif

static unsigned _SysGetColor(Win *w, int colorId)
{
	if (colorId>=0 && colorId<=5) 
		return winColors[colorId];

	switch (colorId) {
	case IC_EDIT_TEXT_BG:	return colorEditTextBg;
	case IC_EDIT_TEXT:	return colorEditText;
	case IC_EDIT_STEXT_BG:	return colorEditSTextBg;
	case IC_EDIT_STEXT:	return colorEditSText;
	case IC_EDIT_DTEXT_BG:	return colorEditDisabledTextBg;
	case IC_EDIT_DTEXT:	return colorEditDisabledText;

	case IC_MENUBOX_BG:	return colorMBoxBg;
	case IC_MENUBOX_BORDER:	return colorMBoxBorder;
	case IC_MENUBOX_TEXT:	return colorMBoxText;
	case IC_MENUBOX_SELECT_BG:	return colorMBoxSelectBg;
	case IC_MENUBOX_SELECT_TEXT:	return colorMBoxSelectText;
	case IC_MENUBOX_SELECT_FRAME:	return colorMBoxSelectFrame;

	case IC_MENUPOPUP_BG: return colorMPBg;
	case IC_MENUPOPUP_LEFT: return colorMPLeft=0xE0E0E0;
	case IC_MENUPOPUP_BORDER: return colorMPBorder=0;
	case IC_MENUPOPUP_LINE: return colorMPLine=0xC0C0C0;
	case IC_MENUPOPUP_TEXT: return colorMPText=0;
	case IC_MENUPOPUP_SELECT_TEXT: return colorMPSText=0;
	case IC_MENUPOPUP_SELECTBG: return colorMPSBg=0xFFA0A0;
	case IC_MENUPOPUP_POINTER: return colorMPPointer=0;
	
	case IC_SCROLL_BORDER: return 0xD0D0D0;
	case IC_SCROLL_BG: return 0xF0F0F0;
	case IC_SCROLL_BUTTON: return 0xD8E9EC; //0xF8F9EC //0xD8E9EC


	default: return 0xFF;
	}
}


static cfont* _SysGetFont(Win *w, int fontId)
{
	return guiFont;
}

unsigned (*SysGetColor)(Win *w, int colorId)=_SysGetColor;
cfont* (*SysGetFont)(Win *w, int fontId) = _SysGetFont;

void Draw3DButtonW2(GC &gc, crect r, unsigned bg, bool up)
{
	static unsigned hp1,lp1;
	static unsigned hp2,lp2;
	static unsigned lastBg = 0;
	static bool initialized = false;

	if (!initialized || lastBg != bg)
	{
		hp1 = ColorTone(bg, -10);
		lp1 = ColorTone(bg, -90);
		hp2 = ColorTone(bg, +150);
		lp2 = ColorTone(bg, -60);
		lastBg = bg;
		initialized = true;
	}

	unsigned php1,plp1,php2,plp2;
	if (up) {
		php1 = hp1;
		plp1 = lp1;
		php2 = hp2;
		plp2 = lp2;
	} else {
		php1 = lp1;
		plp1 = hp1;
		php2 = lp2;
		plp2 = hp2;
	}

	gc.SetLine(plp1);
	gc.MoveTo(r.right-1,r.top);
	gc.LineTo(r.right-1,r.bottom-1);
	gc.LineTo(r.left,r.bottom-1);
	gc.SetLine(php1);
	gc.LineTo(r.left,r.top);
	gc.LineTo(r.right-1,r.top);
	r.Dec();
	gc.MoveTo(r.right-1, r.top);
	gc.SetLine(php2);
	gc.LineTo(r.left, r.top);
	gc.LineTo(r.left, r.bottom-1);
	gc.SetLine(plp2);
	gc.LineTo(r.right-1, r.bottom-1);
	gc.LineTo(r.right-1, r.top);
}


////////////////////////////////////////// StaticLine

cpoint StaticTextSize(GC &gc, const unicode_t *s, cfont *font)
{
	cpoint p(0,0);
	if (font) gc.Set(font);
	while (*s) {
		const unicode_t *t = unicode_strchr(s,'\n');
		int len = (t ? t-s : unicode_strlen(s));

		cpoint c = gc.GetTextExtents(s,len); 
		s +=len;
		if (t) s++;
		if (*s == '\r') s++;
		p.y += c.y;
		if (p.x<c.x) p.x = c.x;
	}
	return p;
}

void DrawStaticText(GC &gc, int x, int y, const unicode_t *s, cfont *font, bool transparent)
{
	if (font) gc.Set(font);
	while (*s) {
		const unicode_t *t = unicode_strchr(s,'\n');
		int len = (t ? t-s : unicode_strlen(s));
		gc.TextOutF(x,y,s,len);
		cpoint c = gc.GetTextExtents(s,len);
		s +=len;
		if (t) s++;
		if (*s == '\r') s++;
		y += c.y;
	}
}

cpoint GetStaticTextExtent(GC &gc, const unicode_t *s, cfont *font)
{
	cpoint res(0,0),c;
	if (font) gc.Set(font);
	while (*s) {
		const unicode_t *t = unicode_strchr(s,'\n');
		int len = (t ? t-s : unicode_strlen(s));

		c = gc.GetTextExtents(s,len);
		s +=len;
		if (t) s++;
		if (*s == '\r') s++;
		res.y += c.y;
		if (res.x < c.x)
			res.x = c.x;
	}
	return res;
}



int StaticLine::GetClassId()
{
	return CI_STATIC_LINE;
}

StaticLine::StaticLine(Win *parent, const unicode_t *txt, crect *rect)
: Win(Win::WT_CHILD, 0, parent, rect), text(new_unicode_str(txt))
{
	if (!rect) 
	{
		GC gc(this);
		SetLSize(LSize(GetStaticTextExtent(gc,txt,GetFont())));
	}
}


void StaticLine::Paint(GC &gc, const crect &paintRect)
{
	crect rect = ClientRect();
	gc.SetFillColor(GetColor(0));
	gc.FillRect(rect); //CCC
	gc.SetTextColor(GetColor(IsEnabled() ? IC_TEXT : IC_GRAY_TEXT)); //CCC
	gc.Set(GetFont());
	DrawStaticText(gc,0,0,text.ptr());
}

StaticLine::~StaticLine(){}


//////////////////////////////////// ToolTip

class TBToolTip: public Win {
	carray<unicode_t> text;
public:
	TBToolTip(Win *parent, int x, int y, const unicode_t *s);
	virtual void Paint(wal::GC &gc, const crect &paintRect);
	virtual int GetClassId();
	virtual ~TBToolTip();
};

int TBToolTip::GetClassId(){ return CI_TOOLTIP; }

TBToolTip::TBToolTip(Win *parent, int x, int y, const unicode_t *s)
:	Win(Win::WT_POPUP, 0, parent),
	text(new_unicode_str(s))
{
	wal::GC gc(this);
	cpoint p = GetStaticTextExtent(gc, s, GetFont());
	Move(crect(x,y,x + p.x + 4, y + p.y + 2));
}

void TBToolTip::Paint(wal::GC &gc, const crect &paintRect)
{
	gc.SetFillColor(GetColor(IC_BG)); //0x80FFFF);
	crect r = ClientRect();
	gc.FillRect(r);
	gc.SetTextColor(GetColor(IC_TEXT));
	//gc.TextOutF(0,0,text.ptr());
	 DrawStaticText(gc, 2, 1, text.ptr(), GetFont(), false);
}

TBToolTip::~TBToolTip(){}

static cptr<TBToolTip> tip;


void ToolTipShow(Win *w, int x, int y, const unicode_t *s)
{
	ToolTipHide();
	if (!w) return;
	crect r = w->ScreenRect();
	tip = new TBToolTip(w, r.left + x, r.top + y, s);
	tip->Enable();
	tip->Show(Win::SHOW_INACTIVE);
}

void ToolTipShow(Win *w, int x, int y, const char *s)
{
	ToolTipShow(w, x, y, utf8_to_unicode(s).ptr());
}

void ToolTipHide()
{
	tip = 0;
}



}; //anmespace wal
