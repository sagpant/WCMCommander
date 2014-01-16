/*
	Copyright (c) by Valery Goryachev (Wal)
*/


#include "nc.h"
#include "ncfonts.h"
#include "ncdialogs.h"
#include "wcm-config.h"
#include "string-util.h"
#include "ltext.h"

bool createDialogAsChild //= false; 
			 = true;

NCShadowWin::NCShadowWin(Win *parent)
: Win(Win::WT_CHILD,0, parent) //, &crect(0,0,300,100))
{

}

void NCShadowWin::Paint(wal::GC &gc, const crect &paintRect)
{
	crect cr = ClientRect();
	gc.SetFillColor(::wcmConfig.whiteStyle ? 0x404040 : 0x01);
	gc.FillRect(cr);
}

NCShadowWin::~NCShadowWin(){}


void NCDialog::CloseDialog(int cmd)
{
	EndModal(cmd);
}


cfont* NCDialog::GetChildFont(Win *w, int fontId)
{
	return dialogFont.ptr();	
}

unsigned NCDialog::GetChildColor(Win *w, int id)
{
	if (w->GetClassId() != CI_BUTTON) 
	{
		if (/*w == &_header && */id == IC_BG) return _bcolor;
		if (/*w == &_header && */id == IC_TEXT) return _fcolor;
	}

	if (w && w->GetClassId() == CI_EDIT_LINE)
	{
		switch (id) {
		case IC_EDIT_TEXT_BG: return (w && w->IsEnabled()) ? (wcmConfig.whiteStyle ? 0xFFFFFF: 0x909000) : _bcolor; //0xA0A000;
		case IC_EDIT_TEXT: return  (w && w->IsEnabled()) ? 0 : 0x808080;
		}

		/*IC_EDIT_STEXT_BG
		IC_EDIT_STEXT
		IC_EDIT_DTEXT_BG
		IC_EDIT_DTEXT*/
	}

	return Win::GetChildColor(w, id);
}

bool NCDialog::Command(int id, int subId, Win *win, void *data)
{
	if (id && win && win->GetClassId() == CI_BUTTON) 
	{
		CloseDialog(id);
		return true;
	}
	return false;
}


NCDialog::NCDialog(bool asChild, NCDialogParent *parent, const unicode_t *headerText, ButtonDataNode *blist, unsigned bcolor, unsigned fcolor)
: OperThreadWin(asChild ? Win::WT_CHILD : WT_MAIN /*WT_POPUP*/,0, parent), //, &crect(0,0,300,100)),
	_shadow(parent),
	_fcolor(fcolor),
	_bcolor(bcolor),
	_header(this, headerText),
	_lo(9,9),
	_buttonLo(3,16),
	_headerLo(3,3),
	_parentLo(3,3),
	enterCmd(0)
{
	Enable();
	
	_lo.SetLineGrowth(4);
	_lo.SetColGrowth(4);
	
	
	_lo.ColSet(0, 2);
	_lo.ColSet(1, 10);
	_lo.ColSet(2, 3);
	
	_lo.ColSet(3, 5);
	_lo.ColSet(5, 5);
	
	_lo.ColSet(6, 3);
	_lo.ColSet(7, 10);
	_lo.ColSet(8, 2);

	_lo.LineSet(0, 2);
	_lo.LineSet(1, 10);
	_lo.LineSet(2, 3);
	
	_lo.LineSet(3, 3);
	//_lo.LineSet(5, 5);
	
	_lo.LineSet(6, 3);
	_lo.LineSet(7, 5);
	_lo.LineSet(8, 2);

	_lo.ColSet(4, 16, 100000);
	_lo.LineSet(4, 16, 100000);

	_lo.AddRect(&_borderRect, 0, 0, 8, 8);
	_lo.AddRect(&_frameRect, 2, 2, 6, 6);

	_headerLo.ColSet(0, 1, 1000);
	_headerLo.ColSet(2, 1, 1000);
	_headerLo.LineSet(0, 2);
	_headerLo.LineSet(2, 2);
	_headerLo.AddWin(&_header, 1, 1);
	_lo.AddLayout(&_headerLo, 1, 4); 

	_buttonLo.ColSet(0, 10, 1000);
	_buttonLo.ColSet(15, 10, 1000);
	_buttonLo.LineSet(0, 2);
	_buttonLo.LineSet(2, 2);
	
	_parentLo.ColSet(0,20,1000);
	_parentLo.ColSet(2,20,1000);
	_parentLo.LineSet(0,20,1000);
	_parentLo.LineSet(2,20,1000);
	_parentLo.AddWin(this,1,1);
	
	_parentLo.SetLineGrowth(0);
	_parentLo.SetLineGrowth(2);

	_parentLo.SetColGrowth(0);
	_parentLo.SetColGrowth(2);

	if (blist)
	{
		int n =0;
		int minW = 0;
		for ( ;blist->utf8text && n<7; blist++, n++)
		{

			cptr<Button> p = new Button(this, utf8_to_unicode( _LT(carray_cat<char>("DB>",blist->utf8text).ptr(), blist->utf8text) ).ptr(), blist->cmd);
			
			p->Show();
			p->Enable();

			if (n>0) 
				_buttonLo.ColSet(n*2, 5);
			_buttonLo.AddWin(p.ptr(), 1, n*2 + 1);

			if (minW<p->GetLSize().x.minimal)
				minW = p->GetLSize().x.minimal;

			_bList.append(p);
//break; ??? 
			
		}

		if (minW>0)
			for (int i = 0; i<_bList.count(); i++)
			{
				LSize s = _bList[i]->GetLSize();	
				s.x.minimal = s.x.maximal = minW;
				_bList[i]->SetLSize(s);
			};

		if (_bList.count()) 
			_bList[0]->SetFocus();

	}

	_lo.AddLayout(&_buttonLo,5,4); 

	_header.Enable();
	_header.Show();

	SetLayout(&_lo);
	
	if (Type()==WT_CHILD && parent) 
		parent->AddLayout(&_parentLo);
	
	
	SetPosition();
	
	SetName(appName);
}

Win* NCDialog::GetDownButton()
{
	return _bList.count() > 0 ? _bList[0].ptr() : 0;
}


void NCDialog::MaximizeIfChild(bool x, bool y)
{
	if (Type()==WT_CHILD && Parent()) {
		_parentLo.SetLineGrowth(0,!y);
		_parentLo.SetLineGrowth(2,!y);
		_parentLo.SetLineGrowth(1,y);
		_parentLo.SetColGrowth(0,!x);
		_parentLo.SetColGrowth(2,!x);
		_parentLo.SetColGrowth(1, x);
		Parent()->RecalcLayouts();
	}
}

void NCDialog::SetPosition()
{

	if (Type() == WT_CHILD) 
	{
		LSize ls = _lo.GetLSize();
//		ls.x.maximal = ls.x.minimal;
//		ls.y.maximal = ls.y.minimal;
		SetLSize(ls);
		if (Parent())
			Parent()->RecalcLayouts();
		return;
	}


       	LSize ls = _lo.GetLSize();

	crect r = Rect();

	int w =  ls.x.ideal>0 ? ls.x.ideal : 20;
	int h =  ls.y.ideal>0 ? ls.y.ideal : 20;

	r.right  = r.left + w;
	r.bottom = r.top + h;

	Win *parent = Parent();
	if (parent)
	{
		crect pr = parent->Rect();

		int w = r.Width();
		int h = r.Height();

		int dx = (pr.Width() - w)/2;
		int dy = (pr.Height() - h)/2;
		

		r.left = pr.left + dx; r.right = r.left+w;
		r.top  = pr.top  + dy; r.bottom = r.top+h;

	}

	Move(r);
}

bool NCDialog::EventClose()
{
	CloseDialog(CMD_CANCEL);
	return false;
}

void NCDialog::EventSize(cevent_size *pEvent)
{
	if (Type()==WT_CHILD && Parent())
	{
		SetPosition();
		crect r = Rect();
		r.left+=10; r.top+=10; r.right+=10; r.bottom+=10;
		if (r != _shadow.Rect()) _shadow.Move(r);
		
	}
	Win::EventSize(pEvent);
}


void NCDialog::EventMove(cevent_move *pEvent)
{
	if (Type()==WT_CHILD && Parent())
	{
		SetPosition();
		crect r = Rect();
		r.left+=10; r.top+=10; r.right+=10; r.bottom+=10;
		if (r != _shadow.Rect()) _shadow.Move(r);
		
	}
	Win::EventMove(pEvent);
}


bool NCDialog::EventKey(cevent_key* pEvent)
{
	if (pEvent->Type() == EV_KEYDOWN && pEvent->Key() == VK_ESCAPE) 
	{
		CloseDialog(CMD_CANCEL);
		return true;
	}; 
	return Win::EventKey(pEvent);
}

int NCDialog::GetFocusButtonNum()
{
	for (int i = 0; i < _bList.count(); i++) 
		if (_bList[i]->InFocus()) return i;
	return -1;
}

bool NCDialog::EventChildKey(Win* child, cevent_key* pEvent)
{
	if (pEvent->Type() == EV_KEYDOWN) 
	{
		if (pEvent->Key() == VK_ESCAPE) 
		{
			CloseDialog(CMD_CANCEL);
			return true;
		} else 
		if (pEvent->Key() == VK_TAB) 
		{
			FocusNextChild();
			return true;
		} else 
		if (pEvent->Key() == VK_RETURN) 
		{
			if (enterCmd && GetFocusButtonNum()<0)
			{
				CloseDialog(enterCmd);
				return true;
			}
		} else 
		if (pEvent->Key() == VK_LEFT || pEvent->Key() == VK_RIGHT)
		{
			int i = GetFocusButtonNum();
			if (i >= 0) {
				int n;
				if (pEvent->Key() == VK_LEFT) 
					n = i ? i - 1 : _bList.count()-1;
				else
					n = ((i+1)%_bList.count());
						
				if (n != i)
					_bList[n]->SetFocus();

				return true;
			}
		}
		
	}; 
 	return Win::EventChildKey(child, pEvent);
}

//namespace wal {
//extern Display* display;
//};

bool NCDialog::EventShow(bool show)
{
	if (show && Type() == WT_CHILD && Parent())
	{
		SetPosition();
		crect r = Rect();
		_shadow.Move(crect(r.left+10,r.top+10,10+r.right,10+r.bottom));
		_shadow.Show(SHOW_INACTIVE);
		_shadow.OnTop();
	}
	OnTop();
	SetFocus();

	return Win::EventShow(show);
}


void NCDialog::Paint(wal::GC &gc, const crect &paintRect)
{
	gc.SetFillColor(_bcolor);
	gc.FillRect(_borderRect);
	Draw3DButtonW2(gc, _borderRect, _bcolor, true);
	crect rect = _frameRect;
	Draw3DButtonW2(gc, _frameRect, _bcolor, false);
//	DrawBorder(gc, rect, 0);
	rect.Dec();
	rect.Dec();
	DrawBorder(gc, rect, 0);
}


NCDialog::~NCDialog()
{
	if (Type()==WT_CHILD && Parent()) 
		((NCDialogParent*)Parent())->DeleteLayout(&_parentLo);
}

/////////////////////////////////// NCVertDialog

bool NCVertDialog::EventChildKey(Win* child, cevent_key* pEvent)
{
	if (pEvent->Type() == EV_KEYDOWN) 
	{
		if (pEvent->Key() == VK_UP || pEvent->Key() == VK_DOWN)
		{
			int n = -1;
			int count = order.count();
			Win *button = GetDownButton();
			if (button) count++;
			
			for (int i = 0; i < order.count(); i++) if (order[i]->InFocus()) { n = i; break; }
			
			if (pEvent->Key() == VK_UP) 
				n = (n + count - 1) % count;
			else if (pEvent->Key() == VK_DOWN) 
				n = (n + 1) % count;
			
			if (n >= 0 && n < order.count()) 
				order[n]->SetFocus();
			else 
				if (button) button->SetFocus();
			
			return true;			
		} 
	}; 
 	return NCDialog::EventChildKey(child, pEvent);
}


NCVertDialog::~NCVertDialog(){}

/*
	//for ltext
	_LT("DB>Ok")
	_LT("DB>Cancel")
	_LT("DB>Yes")
	_LT("DB>No")
*/

char OkButtonText[]="Ok";
char CancelButtonText[] = "Cancel";
char YesButtonText[] = "Yes";
char NoButtonText[] = "No";

ButtonDataNode bListOk[] = { {OkButtonText,CMD_OK}, {0,0}};
ButtonDataNode bListCancel[] = { {CancelButtonText,CMD_CANCEL}, {0,0}};
ButtonDataNode bListOkCancel[] = { {OkButtonText,CMD_OK}, {CancelButtonText,CMD_CANCEL}, {0,0}};
ButtonDataNode bListYesNoCancel[] = { {YesButtonText,CMD_YES}, {NoButtonText,CMD_NO}, {CancelButtonText,CMD_CANCEL}, {0,0}};

char cmdKillTxt[] = " kill ";
char cmdKill9Txt[] = " kill -9 ";

ButtonDataNode bListKill[] = { {CancelButtonText,CMD_CANCEL}, {cmdKillTxt,CMD_KILL}, {cmdKill9Txt,CMD_KILL_9}, {0,0}};


int NCMessageBox(NCDialogParent *parent, const char *utf8head, const char *utf8txt, bool red, ButtonDataNode *buttonList)
{
	carray<unicode_t> str = utf8_to_unicode(utf8txt);
	ccollect<unicode_t> buf;
	
	unicode_t *s = str.ptr();
	int lLine = 0;
	for(;*s;s++)
	{
		if (lLine>100) {
			buf.append('\n');
			lLine=0;
		}
		
		if (*s =='\n') 
			lLine = 0;
			
		buf.append(*s);
		lLine++;
	}
	buf.append(0);
	
	
	NCDialog d(::createDialogAsChild, parent, utf8_to_unicode(utf8head).ptr(), buttonList, red ? 0xFF:0xD8E9EC, red ? 0xFFFFFF :0x1);
	StaticLine text(&d, buf.ptr());

	text.Show();
	text.Enable();
	Layout lo(3,3);
	lo.LineSet(0,5);
	lo.LineSet(2,5);
	lo.ColSet(0,15);
	lo.ColSet(2,15);
	lo.AddWin(&text,1,1);
	d.AddLayout(&lo);

	d.SetPosition();
	return d.DoModal();
}

class GoToDialog: public NCVertDialog {
	EditLine edit;
public:
	GoToDialog(NCDialogParent *parent)
	:	NCVertDialog(::createDialogAsChild, parent, utf8_to_unicode("Go to line").ptr(), bListOkCancel, 0xD8E9EC, 0), 
		edit((Win*)this, 0, 0)
	{
		edit.Enable();
		edit.Show();
		AddWin(&edit);
		order.append(&edit);
		edit.SetFocus();
		SetPosition();
	}
	
	carray<unicode_t> GetText(){ return edit.GetText(); };
		
	virtual ~GoToDialog();
};

GoToDialog::~GoToDialog(){}


int GoToLineDialog(NCDialogParent *parent)
{
	GoToDialog d(parent);
	d.SetEnterCmd(CMD_OK);
	int r = d.DoModal();
	if (r != CMD_OK) return -1;
	carray<unicode_t> str = d.GetText();
	int n=0;
	unicode_t *s = str.ptr();
	while (*s==' ') s++;
	for (;*s>='0' && *s<='9'; s++)
		n=n*10+*s-'0';
	return n;
}

class InputStrDialog: public NCVertDialog {
	EditLine edit;
public:
	InputStrDialog(NCDialogParent *parent, const unicode_t *message, const unicode_t *str)
	:	NCVertDialog(::createDialogAsChild, parent, message, bListOkCancel, 0xD8E9EC, 0),
		edit((Win*)this, 0, 0, 50)
	{
		edit.Enable();
		edit.Show();
		AddWin(&edit);
		order.append(&edit);
		if (str) edit.SetText(str, true);
		edit.SetFocus();
		SetPosition();
	}
	
	carray<unicode_t> GetText(){ return edit.GetText(); };
		
	virtual ~InputStrDialog();
};

InputStrDialog::~InputStrDialog(){}


carray<unicode_t> InputStringDialog(NCDialogParent *parent, const unicode_t *message, const unicode_t *str)
{
	InputStrDialog d(parent, message, str);
	
	d.SetEnterCmd(CMD_OK);
	int r = d.DoModal();
	carray<unicode_t> ret;
	if (r != CMD_OK ) return ret;
	ret = d.GetText();
	return ret;
}

int KillCmdDialog(NCDialogParent *parent, const unicode_t *cmd)
{
	NCDialog d(::createDialogAsChild, parent, utf8_to_unicode("Kill command").ptr(), bListKill, 0xD8E9EC, 0);
	StaticLine text(&d, cmd);

	text.Show();
	text.Enable();
	Layout lo(3,3);
	lo.LineSet(0,5);
	lo.LineSet(2,5);
	lo.ColSet(0,15);
	lo.ColSet(2,15);
	lo.AddWin(&text,1,1);
	d.AddLayout(&lo);

	d.SetPosition();
	return d.DoModal();

}

////////////////////////////////////////////////////////////////////////////

bool CmdHistoryDialog::Command(int id, int subId, Win *win, void *data)
{
	if (id == CMD_ITEM_CLICK && win == &_list)
		EndModal(CMD_OK);
	return NCDialog::Command(id, subId, win, data);
}

CmdHistoryDialog::~CmdHistoryDialog(){}

////////////////////////////////////////////////////////////////////////////

NCDialogParent::~NCDialogParent(){}



////////////////////////////////////////////  DlgMenu



DlgMenuData::DlgMenuData(){}

void DlgMenuData::Add(const unicode_t *name, const unicode_t *comment, int cmd)
{
	Node node;
	if (name) node.name = new_unicode_str(name);
	if (comment) node.comment = new_unicode_str(comment);
	node.cmd = cmd;

	list.append(node);
}

void DlgMenuData::Add(const char *utf8name, const char *utf8coment, int cmd)
{
	Add(utf8name ? utf8_to_unicode(utf8name).ptr() : 0, utf8coment ? utf8_to_unicode(utf8coment).ptr() : 0, cmd);
}

void DlgMenuData::AddSplitter()
{
	Node node;
	node.cmd = 0;
	list.append(node);
}

DlgMenuData::~DlgMenuData(){}


class DlgMenu: public Win {
	DlgMenuData *_data;
	int _current;
	int _itemH;
	int _width;
	int _nameW, _commentW;
	int _splitterH;
public:
	unsigned bgColor;
	unsigned textColor;
	unsigned sBgColor;
	unsigned sTextColor;
	unsigned fcColor;
	unsigned commentColor;
	
	DlgMenu(Win *parent, DlgMenuData *data);

	void SetCurrent(int n, bool searchUp);

	virtual bool EventKey(cevent_key* pEvent);
	virtual void Paint(wal::GC &gc, const crect &paintRect);
	virtual bool EventMouse(cevent_mouse* pEvent);
	virtual bool EventShow(bool show);
	virtual int GetClassId();
	virtual ~DlgMenu();
};


void DlgMenu::SetCurrent(int n, bool searchUp)
{
	int count = _data->Count();
	if (count<=0) { _current = -1; return; }
	if (n>=count) n = count-1;
	if (n<=0) n = 0;

	while (n>=0 && n<count) 
	{
		if (_data->list[n].cmd) break;
		n = n + (searchUp ? -1 : 1);
	}
	_current = n;
}

int DlgMenu::GetClassId()
{
	return CI_BUTTON; //
}

bool DlgMenu::EventShow(bool show)
{
	if (show) SetCapture(); else ReleaseCapture();
	return true;
}


DlgMenu::DlgMenu(Win *parent, DlgMenuData *data)
:	Win(Win::WT_CHILD, Win::WH_TABFOCUS | Win::WH_CLICKFOCUS, parent),
	_data(data),
	_current(0),
	_itemH(16),
	_width(50),
	_nameW(10), _commentW(0),
	_splitterH(5),
	bgColor(::wcmConfig.whiteStyle ? 0xD8E9EC : 0xB0B000),
	textColor(::wcmConfig.whiteStyle ? 0 : 0xFFFFFF),
	sBgColor(0),
	sTextColor(0xFFFFFF),
	fcColor(::wcmConfig.whiteStyle ? 0xFF : 0xFFFF),
	commentColor(::wcmConfig.whiteStyle ? 0x808000 : 0xFFFF00)
{
	wal::GC gc(this);
	gc.Set(GetFont());

	int w = 0;
	int height = 0;

	static unicode_t A[] = {'A','B','C'};
	cpoint p = gc.GetTextExtents(A, 3);
	_itemH = p.y;
	if (_itemH < 16) _itemH = 16;
	_splitterH = _itemH/4;
	if (_splitterH<3) _splitterH = 3;

	for (int i = 0 ; i<_data->Count(); i++)
	{
		unicode_t *name = _data->list[i].name.ptr();

		if (_data->list[i].cmd != 0) 
		{
			if (name) {
				cpoint p = gc.GetTextExtents(name);
				if (_nameW < p.x) _nameW = p.x;
			}

			unicode_t *comment = _data->list[i].comment.ptr();

			if (comment) {
				cpoint p = gc.GetTextExtents(comment);
				if (_commentW < p.x) _commentW = p.x;
			}
			height += _itemH;
		} else 
			height += _splitterH;
	}
	
	w =  16 + 5 + _nameW + 5 + _commentW;
	if (_width < w) _width = w;

	SetLSize(LSize(cpoint(_width, height)) );
}

extern unsigned  UnicodeLC(unsigned ch);

inline bool EqFirst(const unicode_t *s, int c)
{
	return s && UnicodeLC(*s) == c;
}

bool DlgMenu::EventKey(cevent_key* pEvent)
{
	if (_data->Count() && pEvent->Type() == EV_KEYDOWN)
	{
		
		switch (pEvent->Key()) {
		case VK_UP: SetCurrent(_current - 1, true); break;
		case VK_DOWN: SetCurrent(_current + 1, false); break;
		
		case VK_HOME:
		case VK_LEFT: SetCurrent(0, false); break;
		
		case VK_END:
		case VK_RIGHT: SetCurrent(_data->Count()-1, true); ; break;
		
		case VK_RETURN:  
			if (_current>=0 && _current<_data->Count() && _data->list[_current].cmd !=0) 
				Command(_data->list[_current].cmd, 0, this, 0); 
			break;
		default:
			{
				unicode_t c = UnicodeLC(pEvent->Char());
				int i;
				int n = 0;
				int count = _data->Count();

				for (i = 0; i < count; i++) 
					if (EqFirst(_data->list[i].name.ptr(), c)) n++;

				if (n == 1) {
					for (i=0; i < count; i++) 
						if (EqFirst(_data->list[i].name.ptr(), c)) {
							_current = i;
							Command(_data->list[i].cmd, 0, this, 0);
							break;
						};
					break;
				}

				if (n>1) {
					for (i = _current+1; i < count; i++) 
						if (EqFirst(_data->list[i].name.ptr(), c)) goto t;
					for (i = 0; i<=_current && i<count; i++) 
						if (EqFirst(_data->list[i].name.ptr(), c)) goto t;
					t:;
					_current = i;
					break;
				}

			}
			return Win::EventKey(pEvent);
		}
		Invalidate();
	}; 
 	return Win::EventKey(pEvent);
}


bool DlgMenu::EventMouse(cevent_mouse* pEvent)
{
	switch (pEvent->Type())	{
	case EV_MOUSE_MOVE:
		break;

	case EV_MOUSE_PRESS:
	case EV_MOUSE_DOUBLE:
		{
			cpoint p = pEvent->Point();
			int count = _data->Count();
			
			if (!count || p.x < 0 || p.x > _width || p.y < 0) 
			{
				Command(CMD_CANCEL, 0, this, 0); 
				return true;
			}
			
			if (pEvent->Button() != MB_L) break;
			
			int n = -1;
			int h = 0;
			for (int i = 0; i<_data->Count(); i++)
			{
				int ih = _data->list[i].cmd ? _itemH : _splitterH;
				if (p.y < h+ih)
				{
					if (_data->list[i].cmd !=0) n = i;
					break;
				}
				h += ih;
			}

			if (n >= 0) {
				_current = n;
				Invalidate();
				Command(_data->list[n].cmd, 0, this, 0); 
			}
		}
		break;
		
	case EV_MOUSE_RELEASE:
		break;
	};
	return true;
}


void DlgMenu::Paint(wal::GC &gc, const crect &paintRect)
{
	cfont *font = GetFont();
	gc.Set(font);
	int y = 0;

	int count = _data->Count();

	for (int i=0; i < count; i++)
	{
		if (_data->list[i].cmd == 0)
		{
			gc.SetFillColor(bgColor);
			gc.FillRect(crect(0, y, _width, y+_splitterH));
			crect rect(0, y+1, 0+_width, y+2);
			gc.SetFillColor(ColorTone(bgColor, -150));
			gc.FillRect(rect);
			rect.top+=1;
			rect.bottom+=1;
			gc.SetFillColor(ColorTone(bgColor, +50));
			gc.FillRect(rect);
			y += _splitterH;
		} else {
			gc.SetFillColor( i==_current ? sBgColor : bgColor);
			gc.FillRect(crect(0, y, _width, y+_itemH));

			cicon icon;
			icon.Load(_data->list[i].cmd, 16, 16);
			gc.DrawIcon(0, y, &icon);
			gc.SetTextColor(i==_current ? sTextColor : textColor);
			int x = 16 + 5;

			const unicode_t *name = _data->list[i].name.ptr();
			const unicode_t *comment = _data->list[i].comment.ptr();

			if (name) {
				gc.TextOutF(x, y, name);
				if (i!=_current) {
					gc.SetTextColor(fcColor);
					gc.TextOutF(x, y, name, 1);
				}
			}

			if (comment) {
				gc.SetTextColor(i==_current ? sTextColor : commentColor);
				gc.TextOutF(x + _nameW + 5, y, comment);
			}
			y += _itemH;
		}
	}
}

DlgMenu::~DlgMenu(){}


int RunDldMenu(NCDialogParent *parent, const char *header, DlgMenuData *data)
{
	if (!data || data->Count()<=0) return CMD_CANCEL;

	NCDialog dlg(true, parent, utf8_to_unicode(header).ptr(), 0, 
		::wcmConfig.whiteStyle ? 0xD8E9EC : 0xB0B000, 0xFFFFFF);
	DlgMenu mtable(&dlg, data);
	mtable.Show(); 
	mtable.Enable(); 
	mtable.SetFocus();
	dlg.AddWin(&mtable);
	return dlg.DoModal();
}

