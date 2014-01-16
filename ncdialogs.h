/*
	Copyright (c) by Valery Goryachev (Wal)
*/


#ifndef NCDIALOGS_H
#define NCDIALOGS_H

#include "swl.h"
#include "nchistory.h"
#include "operwin.h"

using namespace wal;

class NCShadowWin: public Win {
public:
	NCShadowWin(Win *parent);
	virtual void Paint(wal::GC &gc, const crect &paintRect);
	virtual ~NCShadowWin();
};

struct ButtonDataNode {
	const char *utf8text;
	int cmd;
};


class NCDialogParent: public OperThreadWin {
	Layout _layout;
public:
	NCDialogParent(Win::WTYPE t, unsigned hints=0, Win *_parent = 0, const crect *rect=0)
	:	OperThreadWin(t,hints, _parent, rect), _layout(1,1)
	{
		_layout.SetLineGrowth(0);
		_layout.SetColGrowth(0);
		_layout.ColSet(0,32,100000);
		_layout.LineSet(0,32,100000);
		SetLayout(&_layout); 
	}
	
	void AddLayout(Layout *p){ _layout.AddLayout(p,0,0); RecalcLayouts(); } 
	void DeleteLayout(Layout *p){ _layout.DelObj(p); }
	
	virtual ~NCDialogParent();
};


class NCDialog: public OperThreadWin {
	NCShadowWin _shadow;
	unsigned _fcolor;
	unsigned _bcolor;

	StaticLine _header;
	
	Layout _lo;
	Layout _buttonLo;
	Layout _headerLo;
	
	Layout _parentLo;
	
	crect _borderRect;
	crect _frameRect;

	
	ccollect<cptr<Button> > _bList;
	int enterCmd;
	
protected:		
	Win* GetDownButton();
	int GetFocusButtonNum();
public:
	NCDialog(bool child, NCDialogParent *parent, const unicode_t *headerText, ButtonDataNode *blist, unsigned bcolor = 0xD8E9EC, unsigned fcolor = 0);
	void MaximizeIfChild(bool x = true, bool y=true); //чтоб делать большие дочерние диалоги (типа поиска файлов и тд)
	void SetPosition();
	void AddWin(Win *w){ _lo.AddWin(w,4,4); }
	void AddLayout(Layout *l){ _lo.AddLayout(l,4,4); }
	void SetEnterCmd(int cmd){ enterCmd = cmd; }
	virtual void Paint(wal::GC &gc, const crect &paintRect);
	virtual unsigned GetChildColor(Win *w, int id);
	virtual bool EventShow(bool show);
	virtual bool EventKey(cevent_key* pEvent);
	virtual bool EventChildKey(Win* child, cevent_key* pEvent);
	virtual bool Command(int id, int subId, Win *win, void *data);
	virtual bool EventClose();
	virtual void EventSize(cevent_size *pEvent);
	virtual void EventMove(cevent_move *pEvent);
	virtual void CloseDialog(int cmd);
	virtual cfont *GetChildFont(Win *w, int fontId);
	virtual ~NCDialog();
};


class NCVertDialog: public NCDialog {
protected:
	ccollect<Win*> order;
public:
	NCVertDialog(bool child, NCDialogParent *parent, const unicode_t *headerText, ButtonDataNode *blist, unsigned bcolor = 0xD8E9EC, unsigned fcolor = 0)
	:	NCDialog(child, parent, headerText, blist, bcolor, fcolor){}
	virtual bool EventChildKey(Win* child, cevent_key* pEvent);
	virtual ~NCVertDialog();
};


extern bool createDialogAsChild;
extern ButtonDataNode bListOk[];
extern ButtonDataNode bListCancel[];
extern ButtonDataNode bListOkCancel[];
extern ButtonDataNode bListYesNoCancel[];

enum {
	CMD_KILL = 100,
	CMD_KILL_9 = 101
};

int NCMessageBox(NCDialogParent *parent, const char *utf8head, const char *utf8txt, bool red = false, ButtonDataNode *buttonList = bListOk);
int GoToLineDialog(NCDialogParent *parent);
int KillCmdDialog(NCDialogParent *parent, const unicode_t *cmd);

carray<unicode_t> InputStringDialog(NCDialogParent *parent, const unicode_t *message, const unicode_t *str = 0);


class CmdHistoryDialog: public NCDialog {
	NCHistory &_history;
	int _selected;
	TextList _list;
public:
	CmdHistoryDialog(NCDialogParent *parent, NCHistory &history)
	:	NCDialog(createDialogAsChild, parent, utf8_to_unicode(" History ").ptr(), bListOkCancel), 
		_history(history), 
		_selected(history.Count()-1),
		_list(Win::WT_CHILD,Win::WH_TABFOCUS|WH_CLICKFOCUS, this, VListWin::SINGLE_SELECT, VListWin::BORDER_3D, 0)
	{ 
		for (int i = _history.Count()-1; i>=0; i--) _list.Append(_history[i]); 
		_list.Enable();
		_list.Show();
		_list.SetFocus();
		LSRange h(10, 1000, 10);
		LSRange w(50, 1000, 30);
		_list.SetHeightRange(h); //in characters
		_list.SetWidthRange(w); //in characters
		
		if (_history.Count()>0) {
			_list.MoveFirst(0);
			_list.MoveCurrent(_history.Count()-1);
		}

		
		AddWin(&_list);
		SetEnterCmd(CMD_OK);
		SetPosition();
	};
	
	const unicode_t* Get(){ return _list.GetCurrentString(); }

	virtual bool Command(int id, int subId, Win *win, void *data);
	
	virtual ~CmdHistoryDialog();
};

/*

struct DlgMenuNode {
	const char *str;
	int cmd;
	void Set(const char *s, int c){ str = s; cmd = c; }
};
*/

class DlgMenuData {
	friend class DlgMenu;
	struct Node {
		carray<unicode_t> name;
		carray<unicode_t> comment;
		int cmd;
	};
	ccollect<Node> list;
public:
	DlgMenuData();
	int Count() const { return list.count(); }
/*	const unicode_t *GetName(int n){ return n>=0 && n<list.count() ? list[n].name.ptr() : 0; };
	const unicode_t *GetComment(int n){ return n>=0 && n<list.count() ? list[n].name.ptr(): 0; };
	int GetCmd(int n){ return n>=0 && n<list.count() ? list[n].cmd : 0; };
*/
	void Add(const unicode_t *name, const unicode_t *coment, int cmd);
	void Add(const char *utf8name, const char *utf8coment, int cmd);
	void AddSplitter();
	~DlgMenuData();
};


int RunDldMenu(NCDialogParent *parent, const char *header, DlgMenuData *d);

#endif

