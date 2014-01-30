/*
	Copyright (c) by Valery Goryachev (Wal)
*/


#ifndef WINBASE_H
#define WINBASE_H



namespace wal {

//Class IDs
#define CI_BUTTON	1	
#define CI_SBUTTON	2
#define CI_STATIC_LINE	3
#define CI_EDIT_LINE	4
#define CI_SCROLL_BAR	5
#define CI_VLIST	6
#define CI_TEXT_LIST	7
#define CI_MENUBAR	8
#define CI_POPUPMENU	9
#define CI_TOOLTIP	10

//Color IDs
#define IC_BG		0
#define	IC_FG		1
#define IC_TEXT		2
#define IC_GRAY_TEXT	3
#define IC_FOCUS_MARK	4

//Edit colors
#define IC_EDIT_TEXT_BG		10
#define IC_EDIT_TEXT		11
#define IC_EDIT_STEXT_BG	12
#define IC_EDIT_STEXT		13
#define IC_EDIT_DTEXT_BG	14
#define IC_EDIT_DTEXT		15

//Menu 
#define IC_MENUBOX_BG		20
#define IC_MENUBOX_BORDER	21
#define IC_MENUBOX_TEXT		22
#define IC_MENUBOX_SELECT_BG	23
#define IC_MENUBOX_SELECT_TEXT	24
#define IC_MENUBOX_SELECT_FRAME	25

//PopupMenu
#define IC_MENUPOPUP_BG		30
#define IC_MENUPOPUP_LEFT	31
#define IC_MENUPOPUP_BORDER	32
#define IC_MENUPOPUP_LINE	33
#define IC_MENUPOPUP_TEXT	34
#define IC_MENUPOPUP_SELECT_TEXT	35
#define IC_MENUPOPUP_SELECTBG	36
#define IC_MENUPOPUP_POINTER	37

#define IC_SCROLL_BORDER	40
#define IC_SCROLL_BG		41
#define IC_SCROLL_BUTTON	42

enum BASE_COMMANDS {
	_CMD_BASE_BEGIN_POINT_ = -99,
	CMD_ITEM_CLICK,		//может случаться в объектах с подэлементами при нажатии enter или doubleclick (например VList)
	CMD_ITEM_CHANGED,
	CMD_SBUTTON_INFO,	//with subcommands
	CMD_SCROLL_INFO,		//with subcommands
	CMD_MENU_INFO,		//with subcommands
	CMD_EDITLINE_INFO	//with subcommands
};

void BaseInit();
void Draw3DButtonW2(GC &gc, crect r, unsigned bg, bool up);

extern unicode_t ABCString[];
extern int ABCStringLen;


class Button: public Win {
	bool pressed;
	wal::carray<unicode_t> text;
	cptr<cicon> icon;
	int commandId;

	void SendCommand(){ Command(commandId, 0, this, 0); }
public:
	Button(Win *parent, const unicode_t *txt,  int id, crect *rect = 0, int iconX = 16, int iconY = 16);
	void Set(const unicode_t *txt, int id, int iconX = 16, int iconY = 16);
	int CommandId() const { return commandId; }
	virtual void Paint(GC &gc, const crect &paintRect);
	virtual bool EventFocus(bool recv);
	virtual bool EventMouse(cevent_mouse* pEvent);
	virtual bool EventKey(cevent_key* pEvent);
	virtual void OnChangeStyles();
	virtual int GetClassId();
	virtual ~Button();
};

enum SButtonInfo {
	// subcommands of CMD_SBUTTON_INFO
	SCMD_SBUTTON_CHECKED=1,
	SCMD_SBUTTON_CLEARED=0
};

//Checkbox and Radiobutton
class SButton: public Win {
	bool isSet;
	carray<unicode_t> text;
	int group;
public:
	SButton(Win *parent, unicode_t *txt, int _group, bool _isSet=false, crect *rect = 0);

	bool IsSet(){ return isSet; }
	void Change(bool _isSet);
	virtual void Paint(GC &gc, const crect &paintRect);
	virtual bool EventMouse(cevent_mouse* pEvent);
	virtual bool EventKey(cevent_key* pEvent);
	virtual bool EventFocus(bool recv);
	virtual bool Broadcast(int id, int subId, Win *win, void *data);
	virtual int GetClassId();
	virtual ~SButton();
};


class EditBuf {
	wal::carray<unicode_t> data;
	int size;	//СЂР°Р·РјРµСЂ Р±СѓС„РµСЂР°
	int count;	//РєРѕР»РёС‡РµСЃРІРѕ СЃРёРјРІРѕР»РѕРІ РІ СЃС‚СЂРѕРєРµ
	int cursor;	//РїРѕР·РёС†РёСЏ РєСѓСЂСЃРѕСЂР°
	int marker;	//С‚РѕС‡РєР°, РѕС‚ РєРѕС‚РѕСЂРѕР№ РґРѕ РєСѓСЂСЃРѕСЂР° - РІС‹РґРµР»РµРЅРЅС‹Р№ Р±Р»РѕРє
	
	enum { STEP = 0x100 };
	void SetSize(int n);
	void InsertBlock(int pos, int n);
	void DeleteBlock(int pos, int n);	

public:
	
	void Begin	(bool mark = false)	{ cursor = 0; if (!mark) marker = cursor; }
	void End	(bool mark = false)	{ cursor = count; if (!mark) marker = cursor; }
	void Left	(bool mark = false)	{ if (cursor > 0) cursor--; if (!mark) marker = cursor; }
	void Right	(bool mark = false)	{ if (cursor < count) cursor++; if (!mark) marker = cursor; }
	
	void CtrlLeft	(bool mark = false);
	void CtrlRight	(bool mark = false);

	void SetCursor	(int n, bool mark = false) { if (n > count) n = count-1; if (n<0) n=0; cursor =n; if (!mark) marker = cursor;}
	void Unmark	()			{ marker = cursor; }
	
	bool Marked() const { return cursor != marker; }
	bool DelMarked();
	bool Marked(int n) const 
		{ return cursor != marker && 
			((cursor<marker && n>=cursor && n<marker) || (cursor>marker && n<cursor && n>=marker)); }

	void Insert(unicode_t t);
	void Insert(const unicode_t *txt);
	void Del();
	void Backspace();
	void Set(const unicode_t *s, bool mark = false);
	
	unicode_t* Ptr(){ return data.ptr(); }
	unicode_t operator [](int n){ return data[n]; }
	
	int	Count()  const { return count; }
	int	Cursor() const { return cursor; }
	int	Marker() const { return marker; }
	
	void	Clear(){ marker = cursor = count = 0; }
	
	EditBuf(const unicode_t *s):size(0), count(0){ Set(s);}
	EditBuf():size(0), count(0), cursor(0), marker(0){}
	
	static int GetCharGroup(unicode_t c); //for word left/right (0 - space)
};

enum EditLineCmd {
	// subcommands of CMD_EDITLINE_INFO
	SCMD_EDITLINE_CHANGED = 100
};


class EditLine: public Win {
	EditBuf text;
	int _chars;
	bool cursorVisible;
	bool passwordMode;
	int first;
	bool frame3d;
	void DrawCursor(GC &gc);
	bool CheckCursorPos(); //true - РµСЃР»Рё РЅСѓР¶РЅР° РїРµСЂРµСЂРёСЃРѕРІРєР°
	void ClipboardCopy();
	void ClipboardPaste();
	void ClipboardCut();
	int GetCharPos(cpoint p);
	void Changed(){ if (Parent()) Parent()->Command(CMD_EDITLINE_INFO, SCMD_EDITLINE_CHANGED, this, 0); }
public:
	EditLine(Win *parent, const crect *rect, const unicode_t *txt, int chars = 10, bool frame = true);
	virtual void Paint(GC &gc, const crect &paintRect);
	virtual bool EventMouse(cevent_mouse* pEvent);
	virtual bool EventKey(cevent_key* pEvent);
	virtual void EventTimer(int tid);
	virtual bool EventFocus(bool recv);
	virtual void EventSize(cevent_size *pEvent);
	void Clear();
	void SetText(const unicode_t *txt, bool mark = false);
	void Insert(unicode_t t);
	void Insert(const unicode_t *txt);
	int GetCursorPos(){ return text.Cursor(); }
	void SetCursorPos(int c, bool mark = false){ text.SetCursor(c, mark); }
	carray<unicode_t> GetText();
	void SetPasswordMode(bool enable = true){ passwordMode = enable; Invalidate(); }
	virtual int GetClassId();
	virtual void OnChangeStyles();
	virtual ~EditLine();
};

extern cpoint GetStaticTextExtent(GC &gc, const unicode_t *s, cfont *font);

class StaticLine: public Win {
	wal::carray<unicode_t> text;
public:
	StaticLine(Win *parent, const unicode_t *txt, crect *rect = 0);
	virtual void Paint(GC &gc, const crect &paintRect);
	void SetText(const unicode_t *txt){ text = wal::new_unicode_str(txt); Invalidate(); }
	virtual int GetClassId();
	virtual ~StaticLine();
};

enum ScrollCmd {
	//subcommands of CMD_SCROLL_INFO
	SCMD_SCROLL_VCHANGE=1,
	SCMD_SCROLL_HCHANGE,

	SCMD_SCROLL_LINE_UP,
	SCMD_SCROLL_LINE_DOWN,
	SCMD_SCROLL_LINE_LEFT,
	SCMD_SCROLL_LINE_RIGHT,
	SCMD_SCROLL_PAGE_UP,
	SCMD_SCROLL_PAGE_DOWN,
	SCMD_SCROLL_PAGE_LEFT,
	SCMD_SCROLL_PAGE_RIGHT,
	SCMD_SCROLL_TRACK
};

struct ScrollInfo {
	int size,pageSize,pos;
	ScrollInfo():size(0),pageSize(0),pos(0){}
	ScrollInfo(int _size, int _pageSize, int _pos)
		:size(_size),pageSize(_pageSize),pos(_pos){}
};

class ScrollBar: public Win {
	bool vertical;
	ScrollInfo si;

	int len; //расстояние между крайними кнопками
	int bsize; // размер средней кнопки
	int bpos; // расстояние от средней кнопки до первой

	crect b1Rect, b2Rect, b3Rect;

	bool trace; // состояние трассировки
	int traceBPoint; // точка нажатия мыши от начала средней кнопки (испю при трассировке)

	bool autoHide;

	bool b1Pressed;
	bool b2Pressed;

	Win *managedWin;

	void Recalc(cpoint *newSize=0);
	void SendManagedCmd(int subId, void *data);
public:
	ScrollBar(Win *parent, bool _vertical, bool _autoHide = true,  crect *rect = 0);
	void SetScrollInfo(ScrollInfo *s);
	void SetManagedWin(Win *w = 0){ managedWin =w; SetScrollInfo(0); }
	virtual void Paint(GC &gc, const crect &paintRect);
	virtual bool Command(int id, int subId, Win *win, void *data);
	virtual bool EventMouse(cevent_mouse* pEvent);
	virtual void EventTimer(int tid);
	virtual void EventSize(cevent_size *pEvent);
	virtual int GetClassId();
	virtual ~ScrollBar();
};


class VListWin: public Win {
public:
	enum SelectType { NO_SELECT=0, SINGLE_SELECT, MULTIPLE_SELECT };
	enum BorderType { NO_BORDER=0, SINGLE_BORDER, BORDER_3D};
private:
	SelectType selectType;
	BorderType borderType;

	int itemHeight;
	int itemWidth;

	int xOffset;
	int count;
	int first;
	int current;
	int pageSize;
	int captureDelta;

	unsigned borderColor; //used only for SINGLE_BORDER
	unsigned bgColor; //for 3d border and scroll block

	ScrollBar vScroll;
	ScrollBar hScroll;

	Layout layout;

	crect listRect; //rect в котором надо рисовать список
	crect scrollRect;

	IntList selectList;

protected:
	void SetCount(int);
	void SetItemSize(int h, int w);
	void CalcScroll();
//	void SetBlockLSize(int);
//	void SetBorderColor(unsigned);

	int GetCount(){ return count; }
	int GetItemHeight(){ return itemHeight; }
	int GetItemWidth(){ return itemWidth; }

	void SetCurrent(int);
public:
	VListWin(WTYPE t, unsigned hints, Win *_parent, SelectType st, BorderType bt, crect *rect);
	virtual void DrawItem(GC &gc, int n, crect rect);
	
	void MoveCurrent(int n, bool mustVisible=true);
	void MoveFirst(int n);
	void MoveXOffset(int n);

	int GetCurrent() const { return current; }
	int GetPageFirstItem() const { return first; }
	int GetPageItemCount() const { return pageSize; }


	void ClearSelection();
	void ClearSelected(int n1, int n2);
	void SetSelected(int n1, int n2);
	bool IsSelected(int);

	virtual void Paint(GC &gc, const crect &paintRect);
	virtual void EventSize(cevent_size *pEvent);
	virtual bool EventKey(cevent_key* pEvent);
	virtual bool EventMouse(cevent_mouse* pEvent);
	virtual bool EventFocus(bool recv);
	virtual bool Command(int id, int subId, Win *win, void *data);
	virtual void EventTimer(int tid);
	virtual int GetClassId();
};


struct TLNode{
	int pixelWidth;
	carray<unicode_t> str;
	long intData;
	void *ptrData;
	TLNode() : pixelWidth(-1), intData(0), ptrData(0){}
	TLNode(const unicode_t *s, int i=0, void *p=0) : pixelWidth(-1), str(new_unicode_str(s)), intData(i), ptrData(p) {}
};


typedef cfbarray<TLNode,0x100,0x100> TLList;

class TextList: public VListWin {
	TLList list;
	bool valid;
	int fontH;
	int fontW;
public:
	TextList(WTYPE t, unsigned hints, Win *_parent, SelectType st, BorderType bt, crect *rect);
	virtual void DrawItem(GC &gc, int n, crect rect);
	void Clear();
	void Append(const unicode_t *txt, int i=0, void *p=0);
	void DataRefresh();
	
	const unicode_t* GetCurrentString(){ int cnt = list.count(), c = GetCurrent(); return c<0 || c>=cnt ? 0: list.get(c).str.ptr(); }
	void* GetCurrentPtr(){ int cnt = list.count(), c = GetCurrent(); return c<0 || c>=cnt ? 0: list.get(c).ptrData; }
	int GetCurrentInt(){ int cnt = list.count(), c = GetCurrent(); return c<0 || c>=cnt ? -1: list.get(c).intData; }
	
	void SetHeightRange(LSRange range); //in characters
	void SetWidthRange(LSRange range); //in characters
	virtual int GetClassId();
	virtual ~TextList();
};



enum CmdMenuInfo {
	//subcommands of CMD_MENU_INFO
		SCMD_MENU_CANCEL=0
};

class PopupMenu;

class MenuData {
public:
	enum TYPE {	CMD = 1, SPLIT,	SUB	};
	
	struct Node {
		int type;
		int id;
		wal::carray<unicode_t> leftText;
		wal::carray<unicode_t> rightText;
		MenuData *sub;
		Node():type(0), sub(0){}
		Node(int _type, int _id, const unicode_t *s,  const unicode_t *rt, MenuData *_sub)
			:type(_type), id(_id), sub(_sub)
			{ 
				if (s) leftText = new_unicode_str(s); 
				if (rt) rightText = new_unicode_str(rt); 
			}
	};
	
private:
	friend class PopupMenu;
	
	wal::ccollect<Node> list;
public:
	MenuData();
	int Count(){ return list.count(); }
	void AddCmd(int id, const unicode_t *s, const unicode_t *rt = 0){ Node node(CMD,id,s,rt,0); list.append(node); }
	void AddCmd(int id, const char *s, const char *rt = 0){ AddCmd(id, utf8_to_unicode(s).ptr(), rt ? utf8_to_unicode(rt).ptr() : 0); }
	void AddSplit(){ Node node(SPLIT,0,0,0,0); list.append(node); }
	void AddSub(const unicode_t *s, MenuData *data){ Node node(SUB,0,s,0,data); list.append(node); } 
	void AddSub(const char *s, MenuData *data){ AddSub(utf8_to_unicode(s).ptr(), data); }
	~MenuData(){}
};


class PopupMenu: public Win {
	struct Node {
		MenuData::Node *data;
		crect rect;
		bool enabled;
	};
	wal::ccollect<Node> list;
	int selected;
	wal::cptr<PopupMenu> sub;
	bool SetSelected(int n);
	bool OpenSubmenu();
	Win *cmdOwner;

	bool IsCmd(int n);
	bool IsSplit(int n);
	bool IsSub(int n);
	bool IsEnabled(int n);

	int fontHeight;
	int leftWidth;
	int rightWidth;

	void DrawItem(GC &gc, int n);
public:
	PopupMenu(Win*parent, MenuData *d, int x, int y, Win *_cmdOwner=0);
	virtual bool EventMouse(cevent_mouse* pEvent);
	virtual bool EventKey(cevent_key* pEvent);
	virtual void Paint(GC &gc, const crect &paintRect);
	virtual bool Command(int id, int subId, Win *win, void *data);
	virtual int GetClassId();
	virtual ~PopupMenu();
};

int DoPopupMenu(Win *parent, MenuData *d, int x, int y);

class MenuBar: public Win {
	struct Node {
		wal::carray<unicode_t> text;
		MenuData *data;
	};
	
	wal::ccollect<Node> list;
	wal::ccollect<crect> rectList;
	int select;
	int lastMouseSelect;
	wal::cptr<PopupMenu> sub;

	crect ItemRect(int n);
	void InvalidateRectList(){ rectList.clear(); }
	void DrawItem(GC &gc, int n);
	void SetSelect(int n);
	void OpenSub();
	void Left() { SetSelect( select <= 0 ? (list.count()-1) : (select - 1) ); }
	void Right() { SetSelect( select == list.count()-1  ? 0 : select + 1 ); }
	int GetPointItem(cpoint p);
public:
	MenuBar(Win *parent, crect *rect = 0);
	void Paint(GC &gc, const crect &paintRect);
	virtual bool EventMouse(cevent_mouse* pEvent);
	virtual bool EventKey(cevent_key* pEvent);
	virtual bool Command(int id, int subId, Win *win, void *d);
	virtual bool EventFocus(bool recv);
	virtual void EventEnterLeave(cevent *pEvent);
	virtual void EventSize(cevent_size *pEvent);
	virtual int GetClassId();
	virtual void OnChangeStyles();

	void Clear(){ list.clear(); InvalidateRectList(); }
	void Add(MenuData *data, const unicode_t *text);
	virtual ~MenuBar();
};


//ToolTip РѕРґРёРЅ РЅР° РІСЃРµ РїСЂРёР»РѕР¶РµРЅРёРµ, РїРѕСЌС‚РѕРјСѓ СѓСЃС‚Р°РЅРѕРІРєР° РЅРѕРІРѕРіРѕ, СѓРґР°Р»СЏРµС‚ РїСЂРµРґС‹РґСѓС‰РёР№
void ToolTipShow(Win *w, int x, int y, const unicode_t *s);
void ToolTipShow(int x, int y, const char *s);
void ToolTipHide();


}; // namespace wal


#endif
