/*
	Copyright (c) by Valery Goryachev (Wal)
*/

#include "wcm-config.h"
#include "ncfonts.h"
#include "ncwin.h"
#include "panel.h"
#include "ext-app.h"
#include "color-style.h"
#include "string-util.h"

#include "icons/folder3.xpm"
#include "icons/folder.xpm"
//#include "icons/executable.xpm"
#include "icons/executable1.xpm"
#include "icons/monitor.xpm"
#include "icons/workgroup.xpm"
#include "icons/run.xpm"

#include "vfs-smb.h"
#include "ltext.h"

PanelSearchWin::PanelSearchWin(PanelWin *parent, cevent_key *key)
:	Win(Win::WT_CHILD, 0, parent),
	_parent(parent),
	_edit(this, 0, 0, 16, true),
	_static(this, utf8_to_unicode( _LT("Search:") ).ptr()), 
	_lo(3, 4)
{
	_lo.AddWin(&_static, 1, 1);
	_lo.AddWin(&_edit, 1, 2);
	_lo.LineSet(0,2);
	_lo.LineSet(2,2);
	_lo.ColSet(0,2);
	_lo.ColSet(3,2);
	_edit.Show(); _edit.Enable();
	_static.Show(); _static.Enable();
	SetLayout(&_lo);
	RecalcLayouts();
		
	LSize ls = _lo.GetLSize();
	ls.x.maximal = ls.x.minimal;
	ls.y.maximal = ls.y.minimal;
	SetLSize(ls);
	
	if (_parent) _parent->RecalcLayouts();
	
	if (key) _edit.EventKey(key);
}

void PanelSearchWin::Paint(wal::GC &gc, const crect &paintRect)
{
	crect r = ClientRect();
	Draw3DButtonW2(gc, r, 0xD8E9EC, true);
	r.Dec();
	r.Dec();
	gc.SetFillColor(0xD8E9EC);
	gc.FillRect(r);
}


cfont* PanelSearchWin::GetChildFont(Win *w, int fontId)
{
	return dialogFont.ptr();	
}


unsigned PanelSearchWin::GetChildColor(Win *w, int id)
{
	if (w && w->GetClassId() == CI_EDIT_LINE)
	{
		switch (id) {
		case IC_EDIT_TEXT_BG: return 0x909000; //0xA0A000;
		case IC_EDIT_TEXT: return 0;
		}
	}

	return Win::GetChildColor(w, id);
}

bool PanelSearchWin::Command(int id, int subId, Win *win, void *data)
{
	if (id == CMD_EDITLINE_INFO && subId == SCMD_EDITLINE_CHANGED)
	{
		carray<unicode_t> text = _edit.GetText();
		
		if (!_parent->Search(text.ptr()))
		{
			unicode_t empty = 0;
			_edit.SetText(oldMask.ptr() ? oldMask.ptr() : &empty);
		} else 
			oldMask = text;
		
		return true;
	}
	return false;
}


bool PanelSearchWin::EventKey(cevent_key* pEvent)
{
	return EventChildKey(0,pEvent);
}

void PanelSearchWin::EndSearch(cevent_key* pEvent)
{
	EndModal(0);
	if (pEvent)
		ret_key = new cevent_key(*pEvent);
}


bool PanelSearchWin::EventMouse(cevent_mouse* pEvent)
{
	switch (pEvent->Type())	{
	case EV_MOUSE_DOUBLE:
	case EV_MOUSE_PRESS:	
	case EV_MOUSE_RELEASE:
		ReleaseCapture();
		EndSearch(0);
		break;
	};
	return true;
}


bool PanelSearchWin::EventChildKey(Win* child, cevent_key* pEvent)
{
	if (pEvent->Type() != EV_KEYDOWN) return false;
	
	wchar_t c = pEvent->Char();
	if (c && c>=0x20) return false;
	
	switch (pEvent->Key()) {
		case VK_LCONTROL:
		case VK_LSHIFT:
		case VK_LMENU:
		case VK_RCONTROL:
		case VK_RSHIFT:
		case VK_RMENU:
		case VK_BACK: 
		case VK_DELETE:
			return false;
	}
	
	EndSearch(pEvent);
	
	return true;
}

bool PanelSearchWin::EventShow(bool show)
{
	if (show) {
		LSize ls = _lo.GetLSize();
		ls.x.maximal = ls.x.minimal;
		ls.y.maximal = ls.y.minimal;
		SetLSize(ls);
		crect r = Rect();
		r.right = r.left + ls.x.minimal;
		r.bottom = r.top + ls.y.minimal;
		Move(r, true);
		_edit.SetFocus();
		SetCapture();
	} else
		if (IsCaptured()) 
			ReleaseCapture();
	return true;
}

PanelSearchWin::~PanelSearchWin(){}


cptr<cevent_key> PanelWin::QuickSearch(cevent_key *key)
{
	_search = new PanelSearchWin(this, key);
	crect r = _footRect;
	LSize ls = _search->GetLSize();
	r.right = r.left + ls.x.minimal;
	r.bottom = r.top + ls.y.minimal;
	_search->Move(r);
	
	_search->DoModal();
	
	cptr<cevent_key> ret = _search->ret_key;
	_search = 0;
	
	return ret;
}

static bool accmask_nocase_begin(const unicode_t *name, const unicode_t *mask)
{
	if (!*mask) 
		return *name == 0;
	
	while (true) 
	{
		switch (*mask) {
		case 0:
			return true;
		case '?': break;
		case '*':
			while (*mask == '*') mask++;
			if (!*mask) return true;
			for (; *name; name++ )
				if (accmask_nocase_begin(name, mask)) 
					return true;
			return false;
		default:
			if (UnicodeLC(*name) != UnicodeLC(*mask)) 
				return false; 
		}
		name++; mask++;
	}
}


bool PanelWin::Search(unicode_t *mask)
{
	int cur = Current();
	int cnt = Count();
	
	int i;
	
	for (i = cur; i < cnt; i++)
	{
		const unicode_t *name = _list.GetFileName(i);;
		if (name && accmask_nocase_begin(name, mask)) {
			SetCurrent(i);
			return true;
		}
	}
	
	for (i = 0; i < cnt-1; i++)
	{
		const unicode_t *name = _list.GetFileName(i);
		if (name && accmask_nocase_begin(name, mask)) {
			SetCurrent(i);
			return true;
		}
	}
	return false;
}


void PanelWin::SortByName()
{
	FSNode *p = _list.Get(_current);
	if (_list.SortMode() == PanelList::SORT_NAME) 
		_list.Sort(PanelList::SORT_NAME, !_list.AscSort());
	else
		_list.Sort(PanelList::SORT_NAME, true);
	if (p) SetCurrent(p->Name());
	Invalidate();
}

void PanelWin::SortByExt()
{
	FSNode *p = _list.Get(_current);
	if (_list.SortMode()==PanelList::SORT_EXT) 
		_list.Sort(PanelList::SORT_EXT, !_list.AscSort());
	else
		_list.Sort(PanelList::SORT_EXT, true);
	if (p) SetCurrent(p->Name());
	Invalidate();
}

void PanelWin::SortBySize()
{
	FSNode *p = _list.Get(_current);
	if (_list.SortMode()==PanelList::SORT_SIZE) 
		_list.Sort(PanelList::SORT_SIZE, !_list.AscSort());
	else
		_list.Sort(PanelList::SORT_SIZE, true);
	if (p) SetCurrent(p->Name());
	Invalidate();
}

void PanelWin::SortByMTime()
{
	FSNode *p = _list.Get(_current);
	if (_list.SortMode()==PanelList::SORT_MTIME) 
		_list.Sort(PanelList::SORT_MTIME, !_list.AscSort());
	else
		_list.Sort(PanelList::SORT_MTIME, true);
	if (p) SetCurrent(p->Name());
	Invalidate();
}


void PanelWin::DisableSort()
{
	FSNode *p = _list.Get(_current);
	_list.DisableSort();
	if (p) SetCurrent(p->Name());
	Invalidate();
}

FSString PanelWin::UriOfDir()
{
	FSString s; 
	FS *fs = _place.GetFS(); 
	if (!fs) return s; 
	return fs->Uri(GetPath()); 
}
	
FSString PanelWin::UriOfCurrent()
{
	FSString s; 
	FS *fs = _place.GetFS(); 
	if (!fs) return s; 
	FSPath path = GetPath();
	FSNode *node = GetCurrent();
	if (node) path.PushStr(node->Name());
	return fs->Uri(path);
}



int PanelWin::GetClassId()
{
	return CI_PANEL;
}

unicode_t PanelWin::dirPrefix[]={'/',0};
unicode_t PanelWin::exePrefix[]={'*',0};

inline int GetTextW(wal::GC &gc, unicode_t *txt)
{
	if (!txt || !*txt) return 0;
        return gc.GetTextExtents(txt).x;
}

void PanelWin::OnChangeStyles()
{
	wal::GC gc((Win*)0);
	gc.Set(GetFont());
	_letterSize[0] = gc.GetTextExtents(ABCString);
	_letterSize[0].x /= ABCStringLen;
	
	gc.Set(GetFont(1));
	_letterSize[1] = gc.GetTextExtents(ABCString);
	_letterSize[1].x /= ABCStringLen;


	dirPrefixW = GetTextW(gc, dirPrefix);
	exePrefixW = GetTextW(gc, exePrefix);

	_itemHeight = _letterSize[0].y;

	int headerH = _letterSize[1].y + 2 + 2;
	if (headerH<10) headerH=10;
	_lo.LineSet(1,headerH);
	_lo.LineSet(2,1);
	
	int footH = _letterSize[1].y*3 + 3 + 3;

//	if (footH<20) footH=20;

	_lo.LineSet(5, footH);
	_lo.LineSet(4, 1);


	if (_itemHeight <= 16) //чтоб иконки влезли
		_itemHeight = 16;

	RecalcLayouts(); //получается двойной пересчет, но хер ли делать
		
//как в EventSize (подумать)	
	Check();
	bool v = _scroll.IsVisible();
	SetCurrent(_current);
	if (v != _scroll.IsVisible())
		Check();
}

static int* CheckMode(int *m)
{
	switch (*m) {
	case PanelWin::BRIEF:
	case PanelWin::MEDIUM:
	case PanelWin::FULL:
	case PanelWin::FULL_ST:
	case PanelWin::FULL_ACCESS:
	case PanelWin::TWO_COLUMNS:
		break;
	default:
		*m = PanelWin::MEDIUM;
	}
	return m;
}

PanelWin::PanelWin(Win *parent, int *mode)
:	
	NCDialogParent(WT_CHILD, 0, parent),
	_lo(7,4),
	_scroll(this, true), //, false), //bug with autohide and layouts
	_itemHeight(1),
	_rows(0),
	_cols(0),
	_first(0),
	_current(0),
	_viewMode(CheckMode(mode)), //MEDIUM),
	_inOperState(false),
	_operData((NCDialogParent*)parent),
	_list( ::wcmConfig.panelShowHiddenFiles, ::wcmConfig.panelCaseSensitive)
{
	_lo.SetLineGrowth(3);
	_lo.SetColGrowth(1);

	_lo.ColSet(0,4);
	_lo.ColSet(3,4);

	_lo.LineSet(0, 4);
	_lo.LineSet(6, 4);

	_lo.ColSet(1,32,100000);
	_lo.LineSet(3,32,100000);

	_lo.AddRect(&_headRect, 1, 1, 1, 2);
	_lo.AddRect(&_centerRect, 3, 1);
	_lo.AddRect(&_footRect, 5, 1, 5, 2);
	
	_lo.AddWin(&_scroll, 3, 2);
		
	
	OnChangeStyles();
	
	_scroll.SetManagedWin(this);
	_scroll.Enable();
	_scroll.Show(Win::SHOW_INACTIVE);
	
	NCDialogParent::AddLayout(&_lo); 


	try {
		FSPath path;
		FSPtr fs =  ParzeCurrentSystemURL(path);
		LoadPath(fs, path, 0,0, SET);
	} catch (...) {
	}
}

bool PanelWin::IsSelectedPanel()
{
	NCWin *p = (NCWin*)Parent();
	return p && p->_panel == this;
}

void PanelWin::SelectPanel()
{
	NCWin *p = (NCWin*)Parent();
	if (p->_panel == this) return;
	PanelWin *old = p->_panel;
	p->_panel = this;
	old->Invalidate();
	Invalidate();
}


void PanelWin::SetScroll()
{
	ScrollInfo si;
	si.pageSize = _rectList.count();
	si.size = _list.Count();
	si.pos = _first;
	bool visible = _scroll.IsVisible();
	_scroll.Command(CMD_SCROLL_INFO, SCMD_SCROLL_VCHANGE, this, &si);

	if (visible != _scroll.IsVisible() )
	{
		this->RecalcLayouts();
		Check();
	}
}


bool PanelWin::Command(int id, int subId, Win *win, void *data)
{
	if (id != CMD_SCROLL_INFO) 
		return false;

	int n = _first;
	int pageSize = _rectList.count();
	switch (subId) {
	case SCMD_SCROLL_LINE_UP: n--; break;
	case SCMD_SCROLL_LINE_DOWN: n++; break;
	case SCMD_SCROLL_PAGE_UP: n-=pageSize; break;
	case SCMD_SCROLL_PAGE_DOWN: n+=pageSize; break;
	case SCMD_SCROLL_TRACK: n = ((int*)data)[0]; break;
	}

	if (n + pageSize > _list.Count()) 
		n = _list.Count()-pageSize;

	if (n<0) n = 0;

	if (n != _first) 
	{
		_first = n;
		SetScroll();
		Invalidate();
	}
	return true;
}

bool PanelWin::Broadcast(int id, int subId, Win *win, void *data)
{
	if (id == ID_CHANGED_CONFIG_BROADCAST) 
	{
		FSNode *node = GetCurrent();
		FSString s;
		if (node) s.Copy(node->Name());
	
		bool a = _list.SetShowHidden(wcmConfig.panelShowHiddenFiles);
		bool b = _list.SetCase(wcmConfig.panelCaseSensitive);

//		if (a || b)
//		{

		SetCurrent(_list.Find(s));
		Invalidate();

//		}
		
		return true;
	} 
	return false;
}


#define VLINE_WIDTH (3)
#define MIN_BRIEF_CHARS 12
#define MIN_MEDIUM_CHARS 18 

void PanelWin::Check()
{
	int width = _centerRect.Width();
	int height = _centerRect.Height();

	int cols;
	
	CheckMode(_viewMode);
	
	switch (*_viewMode) {
	case FULL:
	case FULL_ST:
	case FULL_ACCESS:
		cols = 1;
		break;
	case TWO_COLUMNS:
		cols = 2;
		break;
	default:
		{
			cols = 100;
			int minChars = (*_viewMode == BRIEF) ? MIN_BRIEF_CHARS : MIN_MEDIUM_CHARS;
	       
			if (width < cols * minChars * _letterSize[0].x + (cols-1)*VLINE_WIDTH)
			{
				cols = (width + VLINE_WIDTH)/(minChars*_letterSize[0].x + VLINE_WIDTH);
				if (cols<1) cols = 1;
			}
	
			if (*_viewMode != BRIEF && cols>3) cols = 3;
		}
	};
	
	int rows = height/_itemHeight;
	if (rows < 1) rows = 1;               
	
	if (rows>100) rows = 100;
	if (cols>100) cols = 100;
	
	_cols = cols;
	_rows = rows;

	crect rect(0,0,0,0);;
	_rectList.clear();
	_rectList.append_n(rect, rows * cols);
	_vLineRectList.clear();
	_vLineRectList.append_n(rect, cols-1);
	_emptyRectList.clear();
	_emptyRectList.append_n(rect, cols);

	int wDiv = (width-(cols-1)*VLINE_WIDTH)/cols;
	int wMod = (width-(cols-1)*VLINE_WIDTH)%cols;

	int x = _centerRect.left;
	int n = 0;

	for (int c = 0; c < cols; c++)
	{
		int w = wDiv;
		if (wMod)
		{
			w++;
			wMod--;
		}

		int y = _centerRect.top;
		for (int r = 0; r<rows; r++, y+=_itemHeight)
			_rectList[n++] = crect(x,y,x+w, y+_itemHeight);

		_emptyRectList[c] = crect(x, y, x+w, _centerRect.bottom);

		if (c < _vLineRectList.count())
			_vLineRectList[c] = crect(x+w, _centerRect.top, 
				x+w+VLINE_WIDTH, _centerRect.bottom);

		x += w+VLINE_WIDTH;
	}
}

void PanelWin::EventSize(cevent_size *pEvent)
{
	NCDialogParent::EventSize(pEvent);
	Check();
	bool v = _scroll.IsVisible();
	SetCurrent(_current);
	if (v != _scroll.IsVisible())
		Check();
	if (_search.ptr())
	{
		crect r = _footRect;
		LSize ls = _search->GetLSize();
		r.right = r.left + ls.x.minimal;
		r.bottom = r.top + ls.y.minimal;
		_search->Move(r);
	}
}


static const int timeWidth = 21; //in characters
static const int sizeWidth = 10;
static const int minFileNameWidth = 7;

static const int userWidth = 10;
static const int groupWidth = 10;
static const int accessWidth= 13;


#define PANEL_ICON_SIZE 16

cicon folderIcon(xpm16x16_Folder, PANEL_ICON_SIZE, PANEL_ICON_SIZE);
cicon folderIconHi(xpm16x16_Folder_HI, PANEL_ICON_SIZE, PANEL_ICON_SIZE);

static cicon executableIcon(xpm16x16_Executable, PANEL_ICON_SIZE, PANEL_ICON_SIZE);
static cicon serverIcon(xpm16x16_Monitor, PANEL_ICON_SIZE, PANEL_ICON_SIZE);
static cicon workgroupIcon(xpm16x16_Workgroup, PANEL_ICON_SIZE, PANEL_ICON_SIZE);
static cicon runIcon(xpm16x16_Run, PANEL_ICON_SIZE, PANEL_ICON_SIZE);

bool panelIconsEnabled = true;

void PanelWin::DrawItem(wal::GC &gc,  int n)
{
	bool active = IsSelectedPanel() && n == _current;
	int pos = n - _first;
	if (pos<0 || pos>=_rectList.count()) return;

	FSNode *p = (n<0 || n>=_list.Count()) ? 0 : _list.Get(n);

	bool isDir = p && p->IsDir();
	bool isExe = !isDir && p && p->IsExe();
	bool isBad = p && p->IsBad();
	bool isSelected = p && p->IsSelected();
	bool isHidden = p && p->IsHidden();	
	
	PanelItemColors color;
	panelColors->itemF(&color, _inOperState, active, isSelected, isBad, isHidden, isDir, isExe);

	gc.SetFillColor(color.bg);

	
	crect rect = _rectList[pos];
	
	gc.SetClipRgn(&rect);
	gc.FillRect(rect);
	
	if (n<0 || n>=_list.Count()) return;

	if (active) 
		Draw3DButtonW2(gc, rect, color.bg, true);


	int x = rect.left + 7; //5;
	int y = rect.top;

	gc.SetTextColor(color.text);

	if (isDir)
	{
		if (wcmConfig.panelShowIcons) {
			switch (p->extType) {
			case FSNode::SERVER:
				serverIcon.DrawF(gc,x,y);
				break;
				
			case FSNode::WORKGROUP:
				workgroupIcon.DrawF(gc,x,y);
				break;
				
			default: 
				if (( ((color.bg>>16)&0xFF)+((color.bg>>8)&0xFF)+(color.bg&0xFF)) < 0x80*3)
					folderIcon.DrawF(gc,x,y);
				else
					folderIconHi.DrawF(gc,x,y);
			};
		} else {
			gc.TextOutF(x, y, dirPrefix);
			//x += dirPrefixW;
		}
	} else 
	if (isExe)
	{
		if (wcmConfig.panelShowIcons) {
			executableIcon.DrawF(gc,x,y);
		} else {
			gc.TextOutF(x, y, exePrefix);
			//x += exePrefixW;
		}
	} else {
/*	
		if (panelIconsEnabled) {
			ExtNode *en = GetExtNode(_list.GetFileName(n));
			if (en) {
				AppNode *an = GetDefApp(en);
				if (an)
					runIcon.DrawF(gc,x,y);
			}
		}
*/
	}
	
	if (wcmConfig.panelShowIcons) {
		x += PANEL_ICON_SIZE;
	} else {
		x += dirPrefixW;
	}
	
	x += 4;
	
	if (isSelected)
	{
		gc.SetTextColor(color.shadow);
		gc.TextOut(x+1, y+1, _list.GetFileName(n));
		gc.SetTextColor(color.text);
	}
	
	
	if (*_viewMode == FULL_ST) 
	{
		FSNode *p = _list.Get(n);
		
		int width = rect.Width();
		
		int sizeW = sizeWidth*_letterSize[0].x;
		int timeW = timeWidth*_letterSize[0].x;
		
		int sizeX = (width - sizeW - timeW) < minFileNameWidth*_letterSize[0].x ? minFileNameWidth*_letterSize[0].x : width - sizeW - timeW;
		sizeX += rect.left;
		int timeX = sizeX + sizeW;
		
		gc.SetLine(panelColors->modeLine);
		gc.MoveTo(sizeX, rect.top);
		gc.LineTo(sizeX, rect.bottom);
		
		gc.MoveTo(timeX, rect.top);
		gc.LineTo(timeX, rect.bottom);
				
		if (p) 
		{
/*			char cbuf[64];
			unsigned_to_char<seek_t>(p->Size(), cbuf);
			unicode_t ubuf[64];
			latin1_to_unicode( ubuf, cbuf);
			cpoint size = gc.GetTextExtents(ubuf);
			gc.TextOut(sizeX + sizeW - size.x - _letterSize[0].x, y, ubuf);
*/			
			
			unicode_t ubuf[64];
			p->st.GetPrintableSizeStr(ubuf);
			cpoint size = gc.GetTextExtents(ubuf);
			gc.TextOutF(sizeX + sizeW - size.x - _letterSize[0].x, y, ubuf);
		
			unicode_t buf[64]={0};
			p->st.GetMTimeStr(buf);
			gc.TextOutF(timeX + _letterSize[0].x, y, buf);
		}
		
		rect.right = sizeX; 
		gc.SetClipRgn(&rect);
		
	}
	
	
	if (*_viewMode == FULL_ACCESS) 
	{
		FSNode *p = _list.Get(n);
		
		int width = rect.Width();
		
		int accessW = accessWidth*_letterSize[0].x;
		int userW = userWidth*_letterSize[0].x;
		int groupW = groupWidth*_letterSize[0].x;
		
		int accessX = (width - accessW - userW - groupW) < minFileNameWidth*_letterSize[0].x ? minFileNameWidth*_letterSize[0].x : width - accessW - userW - groupW;
		accessX += rect.left;
		int userX = accessX + accessW;
		int groupX = userX + userW;
		
		gc.SetLine(panelColors->modeLine);
		
		gc.MoveTo(accessX, rect.top);
		gc.LineTo(accessX, rect.bottom);
		
		gc.MoveTo(userX, rect.top);
		gc.LineTo(userX, rect.bottom);
		
		gc.MoveTo(groupX, rect.top);
		gc.LineTo(groupX, rect.bottom);

		if (p) 
		{
			unicode_t ubuf[64];
	 		gc.GetTextExtents(p->st.GetModeStr(ubuf));
			gc.TextOutF(accessX + _letterSize[0].x, y, ubuf);
		
			const unicode_t *userName = GetUserName(p->GetUID());
			gc.TextOutF(userX + _letterSize[0].x, y, userName);

			const unicode_t *groupName = GetGroupName(p->GetGID());
			gc.TextOutF(groupX + _letterSize[0].x, y, groupName);

		}
		
		rect.right = accessX; 
		gc.SetClipRgn(&rect);
		
	}

	
	if (active)
	{
		gc.TextOut(x, y, _list.GetFileName(n));
	} else 
		gc.TextOutF(x, y, _list.GetFileName(n));

}

void PanelWin::SetCurrent(int n)
{
	SetCurrent(n, false, 0);
}

void PanelWin::SetCurrent(int n, bool shift, int *selectType)
{
	if (!this) return;

	if (n >= _list.Count()) n = _list.Count()-1;
	if (n < 0) n = 0;
	
//	if (n == _current) return;

	int old = _current;
	_current = n;
	
	bool fullRedraw = false;
	
	if (shift && selectType)
	{
		if (old == _current)
			_list.ShiftSelection(_current, selectType);
		else {
			int count, delta;
			if (old < _current) {
				count = _current-old+1;
				delta = 1;
			} else {
				count = old-_current+1;
				delta = -1;
			}
			for (int i = old; count > 0; count--, i+=delta)
				_list.ShiftSelection(i, selectType);
		}
		fullRedraw = true;
	}

	if (_current >= _first + _rectList.count())
	{
		_first = _current - _rectList.count()+1;
		if (_first<0) _first = 0;
		fullRedraw = true;
	} else
	if (_current<_first)
	{
		_first = _current;
		fullRedraw = true;
	} else 

	if (_list.Count()- _first < _rectList.count() && _first>0)
	{
		_first = _list.Count() - _rectList.count();
		if (_first<0) _first = 0;
		fullRedraw = true;
	}
	
	SetScroll();
	
	if (fullRedraw) 
	{
		Invalidate();
		return;
	}

	wal::GC gc(this);
	gc.Set(GetFont());
	DrawItem(gc, old);
	DrawItem(gc, _current);
	DrawFooter(gc);
}


bool PanelWin::SetCurrent(FSString &name)
{
	int n = _list.Find(name);
	if (n<0) return false;
	SetCurrent(n);
	return true;
}

void SplitNumber_3(char *src, char *dst)
{
	for (int n = strlen(src); n>0; n--)
	{
		if ( (n%3) == 0 ) *(dst++) = ' ';
		*(dst++) = *(src++);
	}
	*dst = 0;
}

void DrawTextRightAligh(wal::GC &gc, int x, int y, const unicode_t *txt, int width)
{
	if (width<=0) return;
	cpoint size = gc.GetTextExtents(txt);
	if (size.x > width)
	{
		crect r(x,y,x+width, y+size.y);
		gc.SetClipRgn(&r);
		x = x + width - size.x;
	}
	
	gc.TextOutF(x, y, txt);
	
	if (size.x > width)
		gc.SetClipRgn();
	
}

void PanelWin::DrawFooter(wal::GC &gc)
{
	unsigned bkColor = panelColors->foot.bg;//0xA00000; //0x800000;
	gc.SetFillColor(bkColor);
	crect tRect = _footRect;
	gc.SetClipRgn(&tRect);

	gc.FillRect(tRect);

	FSNode *cur = GetCurrent();
	gc.Set(GetFont(1));
	
	
	if (_inOperState) {
		unicode_t ub[512];
		utf8_to_unicode( ub, _LT("...Loading...") );
		gc.SetTextColor(panelColors->foot.operText);
		cpoint size = gc.GetTextExtents(ub);
		int x = (tRect.Width()-size.x)/2;
		gc.TextOutF(x, tRect.top + 5, ub);
	
		return;
	};


	{ //print files count and size
		
		PanelCounter selectedCn = _list.SelectedCounter();
		PanelCounter filesCn = _list.FilesCounter();
		int hiddenCount = _list.HiddenCounter().count;
	
		char b1[64];
		unsigned_to_char<long long>(selectedCn.count > 0 ? selectedCn.size : filesCn.size , b1);
		char b11[100];
		SplitNumber_3(b1, b11);
		
		char b3[0x100] = "";
		if (hiddenCount) sprintf(b3, _LT("(%i hidden)"), hiddenCount);
		
		char b2[128];
				if (selectedCn.count)
			sprintf(b2, _LT("%s bytes in %i selected files%s"), b11, selectedCn.count, b3);
		else
			sprintf(b2, _LT("%s bytes in %i files%s"), b11, filesCn.count, b3);
			
			
		unicode_t ub[512];
		
		utf8_to_unicode( ub, b2);
		
		gc.SetTextColor(selectedCn.count ? panelColors->foot.selText : panelColors->foot.curText);
		cpoint size = gc.GetTextExtents(ub);
		int x = (tRect.Width()-size.x)/2;
		if (x < 10) x = 10;
		gc.TextOutF(x, tRect.top + 3, ub);
	}

	if (cur) 
	{
		gc.SetTextColor(panelColors->foot.text);
		
		int sizeTextW = 0;
		
		int y = tRect.top + 3;
		y += _letterSize[1].y;
		
		if (!cur->extType) 
		{
/*		
			char cbuf1[64];
			unsigned_to_char<seek_t>(cur->Size(), cbuf1);
			
			char cbuf[64];
			SplitNumber_3(cbuf1, cbuf);
		
			unicode_t ubuf[64];
			latin1_to_unicode( ubuf, cbuf);
		
			cpoint size = gc.GetTextExtents(ubuf);
			gc.TextOutF(tRect.right-size.x, y, ubuf);
			sizeTextW = size.x;
*/
			unicode_t ubuf[64];
			cur->st.GetPrintableSizeStr(ubuf);
			cpoint size = gc.GetTextExtents(ubuf);
			gc.TextOutF(tRect.right-size.x, y, ubuf);
			sizeTextW = size.x;
		}
	
		const unicode_t *name = cur->GetUnicodeName();
		
		int x = tRect.left + 5;
		DrawTextRightAligh(gc, x, y, name, tRect.right - sizeTextW - x - 15);
		
		//gc.TextOut(tRect.left + 5, y, name);
		
		if (!cur->extType) 
		{
			y += _letterSize[1].y;
			int x = tRect.left + 5;
			unicode_t ubuf[64];
			
#ifdef _WIN32
			FS *pFs = this->GetFS();
			if (pFs && pFs->Type()==FS::SYSTEM)
			{
				int n = 0;
				ubuf[n++] = '[';
				if ((cur->st.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)!=0)ubuf[n++] = 'D';
				if ((cur->st.dwFileAttributes & FILE_ATTRIBUTE_READONLY)!=0)ubuf[n++] = 'R';
				if ((cur->st.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)!=0)ubuf[n++] = 'S';
				if ((cur->st.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)!=0)ubuf[n++] = 'H';
				if ((cur->st.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)!=0)ubuf[n++] =  'A';
				if ((cur->st.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED)!=0)ubuf[n++] =  'E';
				if ((cur->st.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED)!=0)ubuf[n++] =  'C';
				ubuf[n++] = ']';
				ubuf[n] = 0;
			} else 
				cur->st.GetModeStr(ubuf);
#else
			cur->st.GetModeStr(ubuf);
#endif
			cpoint size = gc.GetTextExtents(ubuf);

			gc.TextOutF(x, y, ubuf);
			x += size.x + 10;
#ifdef _WIN32
			if (!(pFs && pFs->Type()==FS::SYSTEM))
#else 
			if (1)
#endif
			{
				const unicode_t *userName = GetUserName(cur->GetUID());
				size = gc.GetTextExtents(userName);
				gc.TextOutF(x, y, userName);

				x += size.x;
				static unicode_t ch=':';
				size = gc.GetTextExtents(&ch,1);
				gc.TextOutF(x, y, &ch, 1);
				x += size.x;		
		
				const unicode_t *groupName = GetGroupName(cur->GetGID());
				gc.TextOutF(x, y, groupName);
			}

			cur->st.GetMTimeStr(ubuf);
			size = gc.GetTextExtents(ubuf);
			gc.TextOutF(tRect.right-size.x, y, ubuf);
		}
	}	
}

void PanelWin::Paint(wal::GC &gc, const crect &paintRect)
{
	crect r1 = ClientRect();
	DrawBorder(gc, r1, panelColors->border1);
	r1.Dec();
	DrawBorder(gc, r1, panelColors->border2);	
	r1.Dec();
	DrawBorder(gc, r1, panelColors->border3);	
	r1.Dec();
	DrawBorder(gc, r1, panelColors->border4);


	gc.Set(GetFont());

	int i;
	for (i = 0; i<_rectList.count(); i++)
		if (paintRect.Cross(_rectList[i])) DrawItem(gc, i + _first);

	gc.SetClipRgn(&r1);

	gc.SetFillColor(panelColors->bg);
	for (i=0; i<_emptyRectList.count(); i++)
		if (paintRect.Cross(_emptyRectList[i]))
			gc.FillRect(_emptyRectList[i]);

	
	for(i=0; i<_vLineRectList.count(); i++)
	if (paintRect.Cross(_vLineRectList[i]))
		{
			gc.SetLine(panelColors->v1);
			gc.MoveTo(_vLineRectList[i].left, _vLineRectList[i].top);
			gc.LineTo(_vLineRectList[i].left, _vLineRectList[i].bottom);
			gc.SetLine(panelColors->v2);
			gc.MoveTo(_vLineRectList[i].left+1, _vLineRectList[i].top);
			gc.LineTo(_vLineRectList[i].left+1, _vLineRectList[i].bottom);
			gc.SetLine(panelColors->v3);
			gc.MoveTo(_vLineRectList[i].left+2, _vLineRectList[i].top);
			gc.LineTo(_vLineRectList[i].left+2, _vLineRectList[i].bottom);
		}

//header
	crect tRect(_headRect.left, _headRect.bottom, _headRect.right, _headRect.bottom+1);
	if (paintRect.Cross(_headRect)) 
	{
		PanelHeaderColors colors;
		panelColors->headF(&colors, IsSelectedPanel());
		
		gc.SetFillColor(panelColors->hLine);

		gc.FillRect(tRect);
	
		gc.SetFillColor(colors.bg);
		tRect = _headRect;
		gc.FillRect(tRect);

		FS *fs =  GetFS();
	
		if (fs) {
			FSPath &path = GetPath();
			FSString uri = fs->Uri(path);
			const unicode_t *uPath = uri.GetUnicode();
		
			cpoint p = gc.GetTextExtents(uPath);
			int x = _headRect.left+2;

			if (p.x > _headRect.Width()) 
				x -= p.x-_headRect.Width();

			gc.SetTextColor(colors.text);
			gc.Set(GetFont(1));
			gc.TextOutF(x, _headRect.top+2, uPath);
		}
	}
	
	tRect = _footRect;
	tRect.bottom = tRect.top;
	tRect.top-=1;
	gc.SetFillColor(panelColors->hLine);
	gc.FillRect(tRect);
	
	if (paintRect.Cross(_footRect))
		DrawFooter(gc);	
}


void PanelWin::LoadPath(FSPtr fs, FSPath &paramPath, FSString *current, cptr<cstrhash<bool,unicode_t> > selected, LOAD_TYPE lType)
{
//printf("LoadPath '%s'\n", paramPath.GetUtf8());
	try {
		StopThread();
		_inOperState = true;
		_operType = lType;
		if (current) _operCurrent = *current; else _operCurrent.Clear();
		_operSelected = selected;
		_operData.SetNewParams(fs, paramPath);
		RunNewThread("Read directory", ReadDirThreadFunc, &_operData); //может быть исключение
		Invalidate();
	} catch (cexception *ex) {
		_inOperState = false;
		NCMessageBox((NCDialogParent*)Parent(), _LT("Read dialog list"), ex->message(), true);
		ex->destroy();
	}
}

void PanelWin::OperThreadSignal(int info)
{
}

void PanelWin::OperThreadStopped()
{
	if (!_inOperState)
	{
		fprintf(stderr, "BUG: PanelWin::OperThreadStopped\n");
		Invalidate();
		return;
	}
	
	_inOperState = false;
	
	try {
		if (!_operData.errorString.IsEmpty())
		{
			Invalidate();
			NCMessageBox((NCDialogParent*)Parent(), _LT("Read dialog list"), _operData.errorString.GetUtf8(), true);
			return;
		}
	
		cptr<FSList> list = _operData.list;
		cptr<cstrhash<bool,unicode_t> > selected = _operSelected;
	
//??? почему-то иногда list.ptr() == 0 ???
//!!! найдено и исправлено 20120202 в NewThreadID стоял & вместо % !!!

		switch (_operType)	{
		case RESET: _place.Reset(_operData.fs, _operData.path); break;
		case PUSH: _place.Set(_operData.fs, _operData.path, true); break;
		default:
			_place.Set(_operData.fs, _operData.path, false); break;
		};
	
		_list.SetData(list);
		if (selected.ptr()) {
			_list.SetSelectedByHash(selected.ptr());
		}
		
		if (!_operCurrent.IsEmpty())
		{
			int n = _list.Find(_operCurrent);
			SetCurrent(n < 0 ? 0 : n);
		} else 
			SetCurrent(0);
			
	} catch (cexception *ex) {
		NCMessageBox((NCDialogParent*)Parent(), _LT("Read dialog list"), ex->message(), true);
		ex->destroy();
	}
	
	Invalidate();
}


void PanelWin::Reread(bool resetCurrent)
{
	cptr<cstrhash<bool,unicode_t> > sHash = _list.GetSelectedHash();
	FSNode *node = resetCurrent ? 0 : GetCurrent();
	FSString s;
	if (node) s.Copy(node->Name());
	
	LoadPath(GetFSPtr(), GetPath(), node ? &s : 0, sHash, RESET);
}


bool PanelWin::EventMouse(cevent_mouse* pEvent)
{
	bool shift = (pEvent->Mod() & KM_SHIFT)!=0;
	bool ctrl = (pEvent->Mod() & KM_CTRL)!=0;

	switch (pEvent->Type())	{
	case EV_MOUSE_MOVE:
		if (IsCaptured()) 
			{
		}
		break;

	
	case EV_MOUSE_DOUBLE:
	case EV_MOUSE_PRESS:
		{
			if (IsSelectedPanel()) 
			{
				if (pEvent->Button()==MB_X1) 
				{
					KeyPrior();
					break;
				}

				if (pEvent->Button()==MB_X2) 
				{
					KeyNext();
					break;
				}
			}

			SelectPanel();

			for (int i = 0; i<_rectList.count(); i++)
				if (_rectList[i].In(pEvent->Point()))
				{
					if (ctrl && pEvent->Button() == MB_L) 
					{
						_list.InvertSelection(_first+i);
						SetCurrent(_first+i);
					} 
					else 
					{
						SetCurrent(_first+i);
						
						if (pEvent->Button() == MB_R) 
						{
							FSNode *fNode =  GetCurrent();
							if (fNode && Current() == _first + i) 
							{
								crect rect = ScreenRect();
								cpoint p = pEvent->Point();
								p.x += rect.left;
								p.y += rect.top;
								((NCWin*)Parent())->RightButtonPressed(p);
							}
						} else
							if (pEvent->Type() == EV_MOUSE_DOUBLE)
								((NCWin*)Parent())->PanelEnter();
					}
					break;
				}

                }
		break;
		

	case EV_MOUSE_RELEASE:
		break;
	};	
	return false;
}

void PanelWin::DirUp()
{
	if (_place.IsEmpty()) return;

#ifdef _WIN32
	{
		FSPtr fs = GetFSPtr();
		if (fs->Type()==FS::SYSTEM)
		{
			FSSys *f = (FSSys*)fs.Ptr();
			if (f->Drive()<0 && GetPath().Count() == 3)
			{
				if (_place.Count() <= 2) {
					static unicode_t aa[]={'\\', '\\', 0};
					carray<wchar_t> name = UnicodeToUtf16(carray_cat<unicode_t>(aa, GetPath().GetItem(1)->GetUnicode()).ptr());

					NETRESOURCEW r;
					r.dwScope = RESOURCE_GLOBALNET;
					r.dwType = RESOURCETYPE_ANY;
					r.dwDisplayType = RESOURCEDISPLAYTYPE_GENERIC;
					r.dwUsage = RESOURCEUSAGE_CONTAINER;
					r.lpLocalName = 0;
					r.lpRemoteName = name.ptr();
					r.lpComment = 0;
					r.lpProvider = 0;
					FSPtr netFs = new FSWin32Net(&r);
					FSPath path(CS_UTF8,"\\");

					LoadPath(netFs, path, 0, 0, RESET);
					return;
				} else {
					if (_place.Pop())
						Reread(true);
					return;
				}
			}
		}
	}
#endif

	
	FSPath p = _place.GetPath();
	
	if (p.Count()<=1)
	{
		if (_place.Pop())
			Reread(true);
		return;
	}
	
	FSString *s = p.GetItem(p.Count()-1);
	FSString item("");
	if (s) item = *s;
	p.Pop();
	LoadPath(GetFSPtr(), p, s ? &item : 0, 0, RESET);
}

void PanelWin::DirEnter()
{
	if (_place.IsEmpty()) return;
	
	if (!Current()) 
	{ 

		DirUp();
		return;
	};
	
	FSNode *node = GetCurrent();
	if (!node || !node->IsDir()) return;
	
	FSPath p = GetPath();
	int cs = node->Name().PrimaryCS();
	p.Push(cs, node->Name().Get(cs));
	
	
	FSPtr fs = GetFSPtr();
	
	if (fs.IsNull()) return;
	
#ifdef _WIN32
	if (fs->Type()==FS::WIN32NET)
	{
		NETRESOURCEW *nr = node->_w32NetRes.Get();
		if (!nr || !nr->lpRemoteName) return;
		
		if (node->extType == FSNode::FILESHARE)
		{
			FSPath path;
			FSPtr newFs = ParzeURI(Utf16ToUnicode(nr->lpRemoteName).ptr(), path, 0, 0);
			LoadPath(newFs, path, 0, 0, PUSH);
		} else {
			FSPtr netFs = new FSWin32Net(nr);
			FSPath path(CS_UTF8,"\\");
			LoadPath(netFs, path, 0, 0, PUSH);
		}
		return;
	}
#endif

	// for smb servers
	
#ifdef LIBSMBCLIENT_EXIST

	if (fs->Type()==FS::SAMBA && node->extType == FSNode::SERVER)
	{
		FSSmbParam param;
		((FSSmb*)fs.Ptr())->GetParam(&param);
		param.SetServer(node->Name().GetUtf8());
		FSPtr smbFs = new FSSmb(&param);
		FSPath path(CS_UTF8,"/");
		LoadPath(smbFs, path, 0, 0, PUSH);
		return;	
	}
#endif	
	
	LoadPath(GetFSPtr(), p, 0, 0, RESET);
}

void PanelWin::DirRoot()
{
	FSPath p;
	p.PushStr(FSString()); //у абсолютного пути всегда пустая строка в начале 
	LoadPath(GetFSPtr(), p, 0, 0, RESET);
}


PanelWin::~PanelWin(){}

