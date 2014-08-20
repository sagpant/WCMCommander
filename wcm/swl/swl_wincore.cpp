/*
	Copyright (c) by Valery Goryachev (Wal)
*/


#include "swl.h"
#include "swl_wincore_internal.h"
#include <string>

namespace wal {

struct WINHASHNODE {
	WINID id;
	Win * win;
	WINHASHNODE(WinID h, Win *w):id(h),win(w){}
	const WINID& key() const { return id; };
private:
	WINHASHNODE():id(0){}
};

static chash<WINHASHNODE, WINID> winhash;
WinID Win::focusWinId = 0;


Win* GetWinByID(WinID hWnd)
{
	WINID id(hWnd);
	WINHASHNODE *node = winhash.get(id);
	return node ? node->win : 0;
}

int DelWinFromHash(WinID w)
{
	WINID id(w);
	winhash.del(id, false);
	return winhash.count();
}

void AddWinToHash(WinID handle, Win *w)
{
	WINHASHNODE node(handle, w);
	winhash.put(node);
}


static void ForeachBlock(WINHASHNODE *t, void *data){	t->win->Block(*((WinID*)data));}
static void ForeachUnblock(WINHASHNODE *t, void *data){	t->win->Unblock(*((WinID*)data));}
void AppBlock(WinID w){	winhash.foreach(ForeachBlock, &w);}
void AppUnblock(WinID w){	winhash.foreach(ForeachUnblock, &w);}


struct TimerStruct {
	WinID winId;
	int timerId;
	unsigned period;
	unsigned last;
};

static ccollect<TimerStruct> timerList;
unsigned GetTickMiliseconds();

static void __TimerAppend(WinID wid, int tid, unsigned period)
{
	if (!period) period = 0;

	TimerStruct ts;
	ts.winId = wid;
	ts.timerId = tid;
	ts.period = period;
	ts.last = GetTickMiliseconds();
	timerList.append(ts);
}

static void TimerDel(WinID wid, int tid)
{
	for (int i = 0, n = timerList.count(); i<n; )
	{
		TimerStruct *ts = timerList.ptr() + i;
		if (ts->winId == wid && ts->timerId == tid)
		{
			timerList.del(i);
			n--;
		}		else i++;
	}
}

static void TimerReset(WinID wid, int tid, unsigned period)
{
	if (!period) period = 0;

	for (int i = 0, n = timerList.count(); i<n; i++)
	{
		TimerStruct *ts = timerList.ptr() + i;
		if (ts->winId == wid && ts->timerId == tid)
		{
			timerList[i].period = period;
			return;
		}
	}
	__TimerAppend(wid, tid, period);
}

void TimerDel(WinID wid)
{
	for (int i = 0; i< timerList.count(); )
	{
		if (timerList[i].winId == wid)
			timerList.del(i);
		else
			i++;
	}
}

struct RunTimersNode {
	WinID wid;
	int tid;
};


unsigned RunTimers()
{

	ccollect<RunTimersNode> runList;
	unsigned tim = GetTickMiliseconds();
	int i,n;
//printf("count timers %i\n", timerList.count());
	for (i = 0, n = timerList.count(); i<n; i++)
	{
		TimerStruct *ts = timerList.ptr() + i;

//printf("tim%i (%i,%i)\n",1, tim, ts->last);

		if (tim - ts->last >= ts->period)  //! условие должно соответствовать условию *
		{
			ts->last = tim;
			RunTimersNode node;
			node.wid = ts->winId;
			node.tid = ts->timerId;
			runList.append(node);
		}
	}

//printf("running timers %i\n", runList.count());
	for (i = 0; i< runList.count(); )
	{
		Win *w=GetWinByID(runList[i].wid);
		if (w) {
			w->EventTimer(runList[i].tid);//...
			i++;
		} else runList.del(i);
	}

	//DWORD 
	unsigned minTim =0xFFFF;
	tim = GetTickMiliseconds();

	for (i = 0, n = timerList.count(); i<n; i++)
	{
		TimerStruct *ts = timerList.ptr() + i;
		//DWORD 
		unsigned t = tim - ts->last;
		if (t >= ts->period) //! условие *
			minTim = 0;
		else {
			//DWORD 
			unsigned m = ts->period - t;
			if (minTim > m)
				minTim = m;
		}
	}
	return minTim;
}


////////////////////////////////////////////////// Win 


void Win::EndModal(int id)
{
	if (modal && !blockedBy) 
		((ModalStruct*)modal)->EndModal(id);
}


void Win::Paint(GC &gc,const crect &paintRect)
{
	gc.SetFillColor(0xA0A0A0);
	gc.FillRect(paintRect);  //CCC
}


bool Win::IsEnabled()
{
	for (Win *p =this; p; p=p->parent) {
		if (!(p->state & S_ENABLED)) return false;
		if (p->IsModal() || p->Type() == Win::WT_MAIN) break;
	}
	return true;
}

bool Win::IsVisible()
{
	return (state & S_VISIBLE) != 0;

/*	for (Win *p =this; p; p=p->parent) 
	{
		if (!(p->state & S_VISIBLE)) return false;
		if (p->Type() == Win::WT_MAIN || p->Type() == Win::WT_POPUP) break;
	}
*/
//	return true;

}

void Win::Enable(bool en)
{
	if (en)
		SetState(S_ENABLED);
	else
		ClearState(S_ENABLED);
	if (IsVisible())
		Invalidate();
}

void Win::SetTimer(int id, unsigned period)
{
	TimerReset/*Append*/(GetID(), id, period);
}

void Win::ResetTimer(int id, unsigned period)
{
	TimerReset(GetID(), id, period);
}

void Win::DelTimer(int id)
{
	TimerDel(GetID(), id);
}

void Win::DelAllTimers()
{
	TimerDel(GetID());
}

bool Win::Event(cevent *pEvent)
{
	switch (pEvent->Type()) {
		case EV_KEYUP:
		case EV_KEYDOWN:
			return EventKey((cevent_key*)pEvent);

		case EV_CLOSE:	return EventClose();
		case EV_SETFOCUS: 
			{
//			Invalidate(); 
			return EventFocus(true);
			}

		case EV_KILLFOCUS:
//			Invalidate(); 
			return EventFocus(false);

		case EV_SHOW: return EventShow(((cevent_show*)pEvent)->Show());

		case EV_MOUSE_PRESS:
			if ( (whint & WH_CLICKFOCUS)!=0 && !InFocus()) 
				SetFocus();
			
		case EV_MOUSE_MOVE:
		case EV_MOUSE_RELEASE:
		case EV_MOUSE_DOUBLE:
			return EventMouse((cevent_mouse*)pEvent);

		case EV_ENTER:
		case EV_LEAVE:
			EventEnterLeave(pEvent);
			return true;

		case EV_SIZE:
			RecalcLayouts();
			EventSize((cevent_size*)pEvent);
			return true;
			
		case EV_MOVE:
			EventMove((cevent_move*)pEvent);
			return true;

		case EV_ACTIVATE:
			return EventActivate(((cevent_activate*)pEvent)->Activated(), ((cevent_activate*)pEvent)->Who());

		default: return false;
	}
}



bool Win::EventMouse(cevent_mouse* pEvent){	return false; }
bool Win::EventKey(cevent_key* pEvent){	return false;}
bool Win::EventChildKey(Win *child, cevent_key* pEvent){	return false;}

bool Win::EventFocus(bool recv)
{ 
	if (recv && lastFocusChild)
	{
		Win *w = GetWinByID(lastFocusChild);
//printf("SET FOCUS TO LAST FOCUS CHILD (%p)\n",w);
		if (w) w->SetFocus();
	}
	
	return false;
}

bool Win::EventClose(){	if (this->IsModal()) EndModal(CMD_CANCEL); return true;}
bool Win::EventShow(bool){	return true; }
void Win::EventTimer(int tid){};
bool Win::EventActivate(bool activated, Win *w){ return false; }
void Win::EventEnterLeave(cevent *pEvent){}
void Win::EventSize(cevent_size *pEvent){}
void Win::EventMove(cevent_move *pEvent){}
void Win::ThreadSignal(int id, int data){}
void Win::ThreadStopped(int id, void* data){}

bool Win::Command(int id, int subId, Win *win, void *data)
{
	return parent ? parent->Command(id, subId,  win, data) : false;
}

bool Win::Broadcast(int id, int subId, Win *win, void *data)
{
	return false;
}
	

int Win::SendBroadcast(int id, int subId, Win *win, void *data, int level )
{
	if (level<=0) return 0;
	level--;
	ccollect<WinID> list;
	int n = 0;
	int i;
	for (i=0; i<childList.count(); i++)
		list.append(childList[i]->GetID());
	for (i=0; i<list.count(); i++)
	{
		Win *w = GetWinByID(list[i]);
		if (w) n += w->SendBroadcast(id, subId, win, data, level);
	}
	if (Broadcast(id, subId, win, data)) n++;
	return n;
}

void Win::SetIdealSize()
{ 
	if (lSize.x.ideal>0 && lSize.y.ideal>0) 
	{ 
		crect r = ScreenRect();
		crect cr= ClientRect();

		int xplus = (r.right-r.left+1)-(cr.right-cr.left+1);
		int yplus = (r.bottom-r.top+1)-(cr.bottom-cr.top+1);
		Move(crect(r.left, r.top, r.left+lSize.x.ideal+xplus,r.top + lSize.y.ideal+yplus)); //???
	}
}


bool Win::UnblockTree(WinID id)
{
	if (blockedBy != id) 
		return false;
	blockedBy = 0;
	for (int i =0; i<childList.count(); i++)
		childList[i]->UnblockTree(id);
	return true;
}


bool Win::IsOneParentWith(WinID h)
{
	Win *w = GetWinByID(h);
	if (!w) return false;
	while (w->Parent()) w = w->Parent();
	Win *t = this;
	while (t->Parent()) t = t->Parent();
	return t == w;
}

void Win::PopupTreeList(ccollect<WinID> &list)
{
	list.append(this->handle);
	for (int i = 0; i<childList.count(); i++) 
		if (childList[i]->type & Win::WT_POPUP)
			childList[i]->PopupTreeList(list);
}

Win *Win::FocusNPChild(bool next)
{
	WinID c = lastFocusChild;
	Win **list = childList.ptr();
	int count = childList.count();
	int i=0;
	Win *p = 0;
	for (i = 0; i<count; i++)
		if (list[i]->handle == c) {
			p = list[i];
			break;
		};
	if (p) {
		int n = i;
		if (next) {
			for (i=n+1; i<count; i++) 
				if ((list[i]->whint &  WH_TABFOCUS) &&list[i]->IsVisible()) 
				{	
					list[i]->SetFocus();
					return list[i];
				}
			for (i=0; i<n; i++) 
				if ((list[i]->whint &  WH_TABFOCUS) &&list[i]->IsVisible()) 
				{	
					list[i]->SetFocus();
					return list[i];
				}
		} else {
			for (i=n-1; i>=0; i--)
				if ((list[i]->whint &  WH_TABFOCUS) &&list[i]->IsVisible()) 
				{	
					list[i]->SetFocus();
					return list[i];
				}
			for (i=count-1; i>n; i--)
				if ((list[i]->whint &  WH_TABFOCUS) &&list[i]->IsVisible()) 
				{	
					list[i]->SetFocus();
					return list[i];
				}
		}
		if (!p->InFocus()) 
			p->SetFocus();
		return p;

	} else {
		for (i=0; i<count; i++)
			if ((list[i]->whint &  WH_TABFOCUS) &&list[i]->IsVisible()) 
			{
				list[i]->SetFocus();
				return list[i];
			}
	}
	return 0;
}


void Win::OnChangeStyles()
{
}
	
void Win::StylesChanged(Win *w)
{
	if (!w) return;
	int count = w->ChildCount();
	for (int i = 0; i<count; i++)
	{
		Win *ch = w->GetChild(i);
		if (!ch) break;
		StylesChanged(ch);
	};
	w->OnChangeStyles();
	w->RecalcLayouts();
	w->Invalidate();
}

cfont* Win::GetChildFont(Win *w, int fontId)
{
	return parent ? parent->GetChildFont(w, fontId) : SysGetFont(w, fontId); 
}

void Win::ThreadCreate(int id, void* (*f)(void*), void* d)
{
	wth_CreateThread(this, id, f, d);
}



unsigned ColorTone(unsigned color, int tone /*0-255*/)
{
	unsigned char R= (unsigned char)(color % 0x100),G=(unsigned char)((color/0x100)%0x100),B = (unsigned char)((color /0x10000)%0x100);
	if (tone>=0) 
	{
		unsigned char t = tone%0x100;
		R += ((0x100-R)*t)/0x100;
		G += ((0x100-G)*t)/0x100;
		B += ((0x100-B)*t)/0x100;
	} else {
		unsigned char t = (-tone) %0x100;
		R -= ((R*t)/0x100);
		G -= ((G*t)/0x100);
		B -= ((B*t)/0x100);
	}
	return ((B%0x100)<<16) + ((G%0x100)<< 8) + (R%0x100);
}

void DrawPixelList(GC &gc, unsigned short*s, int x, int y, unsigned color)
{
	int n = *(s++);
	for (int i =0; i<n; i++, s++, y++)
	{
		unsigned c = *s;
		for (int j=0; j<16; j++, c>>=1)
			if (c&1) gc.SetPixel(x+j,y,color);
	}
}




/////////////////////////////////////// cevent's

cevent::~cevent(){}
cevent_show::~cevent_show(){}
cevent_input::~cevent_input(){}
cevent_key::~cevent_key(){}
cevent_mouse::~cevent_mouse(){}
cevent_activate::~cevent_activate(){}
cevent_size::~cevent_size(){}
cevent_move::~cevent_move(){}

///////////////////////////////////////   Win Threads

WthInternalEvent wthInternalEvent;

enum WTHCMD {
	WTH_CMD_NONE = 0,
	WTH_CMD_EVENT = 1,
	WTH_CMD_END = 2
};

struct WTHNode {
	volatile thread_t th;
	volatile int id;
	Win *volatile w;
	volatile int data;
	void* (*volatile func)(void*);
	void* volatile fData;
	volatile int cmd;
	WTHNode * volatile thNext;
	WTHNode * volatile winNext;
	WTHNode * volatile cmdNext;
	WTHNode():id(0),w(0),data(0),func(0),fData(0),cmd(0),thNext(0),winNext(0),cmdNext(0){}
};

inline unsigned th_key(thread_t th)
{
	unsigned key=0;
	for (int i = 0; i<sizeof(th); i++) key = key + ((unsigned char*)&th)[i];
	return key;
}

inline unsigned win_key(Win *w)
{
	return (unsigned)(w-(Win*)0);
}

class WTHash {
	friend void wth_DropWindow(Win *w);
	friend void wth_CreateThread(Win *w, int id, void* (*f)(void*), void* d);
	friend bool WinThreadSignal(int data);
	friend void *_swl_win_thread_func(void *data);
	friend void wth_DoEvents();

	#define WTH_HASH_SIZE (331)
	WTHNode * volatile thList[WTH_HASH_SIZE];
	WTHNode * volatile winList[WTH_HASH_SIZE];
	WTHNode * volatile cmdFirst;
	WTHNode * volatile cmdLast;
	
	Mutex mutex;            
public:
	WTHash()
	: cmdFirst(0), cmdLast(0)
	{
		int i;
		for (i = 0; i<WTH_HASH_SIZE; i++) thList[i]=0;
		for (i = 0; i<WTH_HASH_SIZE; i++) winList[i]=0;
	}
	~WTHash(){}
} wtHash;


void *_swl_win_thread_func(void *data)
{
	WTHNode *p = ((WTHNode*)data);
	void* ret = p->func(p->fData);

	MutexLock lock(&wtHash.mutex);
	unsigned tN = th_key(p->th) % WTH_HASH_SIZE;

	WTHNode * volatile *t = &(wtHash.thList[tN]);

	while (*t) {
		if (t[0] == p)
		{
			t[0] = p->thNext;
			p->thNext = 0;
			break;
		}
		t = &(t[0]->thNext);
	}
	
	if (!p->cmd) {
		p->cmdNext = 0;
		if (wtHash.cmdLast) 
			wtHash.cmdLast->cmdNext = p;
		else 
			wtHash.cmdFirst = p;

		wtHash.cmdLast = p;
	} 

	p->cmd = WTH_CMD_END;

	try {
		wthInternalEvent.SetSignal(); 
	} catch (cexception *e) { //???
		e->destroy();
	}

	return ret;
}

void wth_CreateThread(Win *w, int id, void* (*f)(void*), void* d)
{
	WTHNode *p = new WTHNode;
	p->w = w;
	p->id = id;
	p->func = f;
	p->fData = d;
	MutexLock lock(&wtHash.mutex);

	int err = thread_create(const_cast<thread_t*>(&(p->th)), _swl_win_thread_func, p);

	if (err) throw_syserr(err,"can`t create window thread");

	unsigned tN = th_key(p->th) % WTH_HASH_SIZE;
	unsigned wN = win_key(w) % WTH_HASH_SIZE;
	p->winNext = wtHash.winList[wN]; wtHash.winList[wN] = p;
	p->thNext =  wtHash.thList[tN]; wtHash.thList[tN] = p;
}

bool WinThreadSignal(int data)
{
	thread_t th = thread_self();
	unsigned n = th_key(th) % WTH_HASH_SIZE;
//printf("Find tn=%i (%p)\n", n, th);
	MutexLock lock(&wtHash.mutex);

	wthInternalEvent.SetSignal();

	for (WTHNode *p = wtHash.thList[n]; p; p=p->thNext)
		if (thread_equal(th, p->th))
		{
			if (!p->cmd) {
				p->cmdNext = 0;
				if (wtHash.cmdLast) 
					wtHash.cmdLast->cmdNext = p;
				else 
					wtHash.cmdFirst = p;

				wtHash.cmdLast = p;
			}
			p->cmd = WTH_CMD_EVENT;
			p->data = data;

			return p->w != 0;
		}
//printf("thread not found %p\n", th);
	return false;
}

void wth_DropWindow(Win *w)
{
	MutexLock lock(&wtHash.mutex);
	unsigned n = win_key(w) % WTH_HASH_SIZE;
	WTHNode * volatile *p = &(wtHash.winList[n]);
	while (*p) {
		if (p[0]->w == w)
		{
			p[0]->w = 0;
			WTHNode *t = p[0];
			p[0] = p[0]->winNext;
			t->winNext = 0;
		} else p = &(p[0]->winNext);
	}
}


void wth_DoEvents()
{
	WTHNode *last = 0;

	{
		MutexLock lock(&wtHash.mutex);
		last = wtHash.cmdLast;
		wthInternalEvent.ClearSignal();
	}

	if (!last) return;

	while (true) 
	{
		MutexLock lock(&wtHash.mutex);
		WTHNode *p = wtHash.cmdFirst;
		if (!p) break; //botva?
		if (p->cmd == WTH_CMD_END)
		{
			if (p->w) 
			{
				unsigned n = win_key(p->w) % WTH_HASH_SIZE;
				for (WTHNode * volatile * pp = &(wtHash.winList[n]); pp[0]; pp = &(pp[0]->winNext))
					if (pp[0] == p)
					{
						pp[0] = p->winNext;
						break;
					}
			}
			wtHash.cmdFirst = p->cmdNext;
			if (!wtHash.cmdFirst) wtHash.cmdLast = 0;
			lock.Unlock();

			try {
				void *ret=0;
				thread_join(p->th, &ret);

				if (p->w != 0) 
					p->w->ThreadStopped(p->id, ret);

//printf("End of Thread Win=%p, id=%i, ret = %p\n", p->w, p->id, ret);

			} catch (...) {
				delete p;
			};
			delete p;

		} else 	if (p->cmd == WTH_CMD_EVENT) 	{
			Win *w = p->w;
			int id = p->id;
			int data = p->data;
			p->cmd = 0;
			wtHash.cmdFirst	= p->cmdNext;
			if (!wtHash.cmdFirst) wtHash.cmdLast = 0;
			lock.Unlock();
			
			if (w) 
				w->ThreadSignal(id, data);

//printf("Thread Evant Win=%p, id=%i, data=%i\n",w,id,data);

		} else {
			fprintf(stderr,"bad state in void wth_DoEvents()\n");
		}

		if (p == last) break;
	}
}


///////////////////////////////////////////// ClipboardText

void ClipboardText::CopyFrom(const ClipboardText&a)
{
	Clear();
	int i;
	for (i = 0; i<a.list.count(); i++)
	{
		carray<unicode_t> p = new unicode_t[BUF_SIZE];
		memcpy(p.ptr(), a.list.const_item(i).const_ptr(), 
			BUF_SIZE * sizeof(unicode_t));
		list.append(p);
	}
	count = a.count;
}

ClipboardText::ClipboardText()
:	count(0)
{
}

ClipboardText::ClipboardText(const ClipboardText&a)
:	count(0)
{
	CopyFrom(a); 
}

ClipboardText& ClipboardText::operator = (const ClipboardText&a)
{
	CopyFrom(a); 
	return *this; 
}


void ClipboardText::Clear()
{
	list.clear(); 
	count = 0; 
}
	
void ClipboardText::Append(unicode_t c)
{
	int n = count + 1;
	int bc = (n + BUF_SIZE - 1) / BUF_SIZE;
	
	while (list.count() < bc) 
	{
		carray<unicode_t> p = new unicode_t[BUF_SIZE];
		list.append(p);
	}
		
	count++;
	n = count - 1;
		
	list[n / BUF_SIZE][n % BUF_SIZE] = c;
}

void ClipboardText::AppendUnicodeStr(const unicode_t* c)
{
	while (c && *c)
	{
		this->Append(*c);
		c++;
	}
}

/////////////////////////////////    Image32 /////////////////////////////

void Image32::alloc(int w, int h)
{
	wal::carray<unsigned32> d(w*h);
	wal::carray<unsigned32*> l(h);
	
	if (d.ptr()) 
	{
		unsigned32 *p = d.ptr();
		for (int i = 0; i<h; i++, p+=w) l[i] = p;
	}
	
	_data = d;
	_lines = l;
	_width = w;
	_height = h;
}

void Image32::clear(){ _width=_height=0; _data.clear(); _lines.clear(); }

void Image32::fill(int left, int top, int right, int bottom,  unsigned c)
{
	if (left < 0) left = 0;
	if (right > _width) right = _width;
	if (top < 0) top = 0;
	if (bottom > _height) bottom = _height;
	if (left >= right || top >= bottom) return;
	
	for (int i = top; i<bottom; i++)
	{
		unsigned32 *p = _lines[i];
		unsigned32 *end = p + right;
		p += left;
		for (;p < end; p++) *p = c;
	}
}

void Image32::copy(const Image32 &a)
{
	if (a._width != _width || a._height != _height) alloc(a._width, a._height);
	if (_width > 0 && _height > 0) 
	{
		memcpy(_data.ptr(), a._data.const_ptr(), _width * _height * sizeof(unsigned32));
		unsigned32 *p = _data.ptr();
		for (int i = 0; i < _height; i++, p += _width) _lines[i] = p;
	}
}


#define FROM_RGBA32(src,R,G,B,A) { A = ((src)>>24)&0xFF; B=((src)>>16)&0xFF; G=((src)>>8)&0xFF; R=(src)&0xFF; }
#define FROM_RGBA32_PLUS(src,R,G,B,A) { A += ((src)>>24)&0xFF; B+=((src)>>16)&0xFF; G+=((src)>>8)&0xFF; R+=(src)&0xFF; }
#define TO_RGBA32(R,G,B,A,n) ((((A)/(n))&0xFF)<<24) | ((((B)/(n))&0xFF)<<16) | ((((G)/(n))&0xFF)<<8) | ((((R)/(n))&0xFF))

void Image32::copy(const Image32 &a, int w, int h)
{
	if (a._width == w && a._height == h) 
	{
		copy(a); 
		return;
	}
	
	if (!w || !h) 
	{
		clear();
		return;
	} 
	
	alloc(w, h);
	
	if (a._width<=0 || a._height<=0) return;
	
	//упрощенная схема, не очень качественно, зато быстро 
	
	carray<int[2]> p(w);
	
	int x;
	int wN = a._width/w;
	int wN2 = wN/2;
	
	for (x=0; x<w; x++)
	{
		int t = (x*a._width)/w;
		int t1 = t - wN2;
		int t2 = t1 + wN;
		if (t2 == t1) t2 = t1+1;
		p[x][0] = (t1 >= 0) ? t1 : 0;
		p[x][1] = (t2 <= a._width) ? t2 : a._width;
	}
	
	int hN = a._height/h;
	
	int y;
	
	for (y=0; y < h; y++)
	{
		
		unsigned32 *line =  _lines[y];
		int hN2 = hN/2;

		if (!hN)
		{
			const unsigned32 *a_line =  a._lines.const_item((y*a._height)/h);
			if (!wN) 
				for (x=0; x<w; x++) line[x] = a_line[p[x][0]];
			else {
				for (x=0; x<w; x++) 
				{
					unsigned R,G,B,A;
					int n1 = p[x][0], n2 = p[x][1];
					FROM_RGBA32(a_line[n1], R, G, B, A);
					for (int i = n1+1; i<n2; i++) FROM_RGBA32_PLUS(a_line[i], R, G, B, A);
					
					n2 -= n1;
					line[x] = n2 ? TO_RGBA32(R, G, B, A, n2) : 0xFF;
				}
			}
		} else {
		
			int t = (y*a._height)/h;
			int t1 = t - hN2;
			int t2 = t1 + hN;
			if (t1 < 0) t1 = 0;
			if (t2 > a._height)  t2 = a._height;

			for (x = 0; x < w; x++) 
			{
			
				unsigned R = 0, G = 0, B = 0, A = 0;
				int n1 = p[x][0], n2 = p[x][1];
			
				for (int iy = t1; iy < t2; iy++)
				{
					const unsigned32 *a_line =  a._lines.const_item(iy);
					for (int i = n1; i < n2; i++) FROM_RGBA32_PLUS(a_line[i],R,G,B,A);
				}
				
				n2 -= n1;
				n2 *= (t2 - t1);
				line[x] = n2 ? TO_RGBA32(R,G,B,A, n2) : 0xFF00;
			}
		}
	}
}

void Image32::copy(const XPMImage &xpm)
{
	const unsigned *colors = xpm.Colors();
	const int *data = xpm.Data();
	clear();
	int w = xpm.Width();
	int h = xpm.Height();
	if (w<=0 || h<=0) return;
	alloc(w, h);
	for (int y=0; y<h; y++)
	{
		unsigned32 *line =  _lines[y];
		for (int x = 0; x<w; x++, line++, data++)
			*line = (*data>=0) ? colors[*data] : 0xFF000000;
	}
}


//////////// XPMImage

void XPMImage::Clear()
{
	colors.clear();
	colorCount = 0;
	none = -1;
	width = height = 0;
	data.clear();
}

struct color_tree_node {
	bool isColor;
	union {
		color_tree_node *p[0x100];
		int c[0x100];
	};
	color_tree_node(bool _is_color)
	:	isColor(_is_color)
	{
		int i;
		if (_is_color) { for (i = 0; i<0x100; i++) c[i]=-1; }
		else { for (i = 0; i<0x100; i++) p[i]=0; }
	}
	~color_tree_node();
};

color_tree_node::~color_tree_node()
{
	if (!isColor)
	{
		for (int i = 0; i<0x100; i++) if (p[i]) delete p[i];
	}
} 

static void InsertColorId(color_tree_node *p, const char *s, int count, int colorId)
{
	if (!p) return;

	while (count>0) 
	{
		unsigned char n = *(s++);
		count--;
		

		if (p->isColor) 
		{

			if (!count) 
				p->c[n] = colorId;  
			//else ignore
				
			return;
		} else {
			if (count) {
				if (!p->p[n]) 
					p->p[n] = new color_tree_node(count==1);
					
				p = p->p[n];
			} else return; //else ignore
		}
	}
}

static int GetColorId(color_tree_node *p, const char *s, int count)
{
	while (p && count>0) 
	{
		unsigned char n = *(s++);
		count--;
		
		if (p->isColor) {
			return p->c[n];
		} else {
			p = p->p[n];
		}
	}
	return -1;
}

inline void SS(const char *&s){ while (*s>0 && *s<=32) s++; }

static int SInt(const char *&s) 
{
	int n = 0;
	for (; *s && *s>='0' && *s<='9'; s++)
		n = n*10 + (*s-'0');
	return n;	 
}

static unsigned SHex(const char *&s) 
{
	unsigned n = 0;
	for (; *s; s++)
	{
		if (*s>='0' && *s<='9')
			n = (n<<4) + (*s-'0');	
		else if (*s>='a' && *s<='f')
			n = (n<<4) + (*s-'a') + 10;
		else if (*s>='A' && *s<='F')
			n = (n<<4) + (*s-'A') + 10;
		else return n;
	}
	return n;	 
}

static unsigned HSVtoRGB(unsigned hsv)
{
	return hsv; //!!! 
}


bool XPMImage::Load(const char **list, int count)
{
	Clear();
	
	if (count<1) {
//printf("err 0\n");	
		return false;
	}

	const char *s = list[0];
	
	list++;
	count--;

	SS(s);
	int w =SInt(s); 
	SS(s);
	int h = SInt(s);
	SS(s);
	int nc = SInt(s);
	SS(s);
	int cpp = SInt(s);
	
	if (!(w >0 && h>0 && nc>0 && cpp>0) || w>16000 || h>16000 || nc>0xFFFFF || cpp>4)
	{
//printf("err 1\n");
		return false;
	}
	
	colors.alloc(nc);
		{
			for (int i= 0; i<nc; i++) colors[i]=0;
		}
	data.alloc(w*h);
		{
			int *p = data.ptr();
			for (int n = w*h; n>0; n--, p++) *p = -1;
		}
		
	colorCount = nc;
	width = w;
	height = h;
	
	
	int i;
	
	color_tree_node tree(cpp==1);
	
	for (i = 0; i<nc && count>0; i++, count--, list++)
	{
		const char *s = list[0];
		int l = strlen(s);
		
		if (l<=cpp) {
			return false;
//printf("err 2\n");
		}
		
		s+=cpp;
		
		
		while (*s) {
			SS(s);
			if (*s=='c')
			{
				s++;
				SS(s);
				if (*s=='#') {
					s++;
					unsigned c = SHex(s);
					colors[i] = (c & 0xFF00FF00) | ((c>>16)&0xFF) | ((c<<16)&0xFF0000);
					
				} else if (*s=='%') {
					s++;
					colors[i] = HSVtoRGB(SHex(s));
				} else if ( 	(s[0]=='N'||s[0]=='n') && 
						(s[1]=='O'||s[1]=='o') &&
						(s[2]=='N'||s[2]=='n') && 
						(s[3]=='E'||s[3]=='e')
						){
					s+=4;
					break;
				} else {
					//ignore
					while (*s<0 || *s>32) s++;
				}
				InsertColorId(&tree, list[0], cpp, i);
				
				break;
			} else 
				break; //ignore
		};
				
		
	}
	
	int *dataPtr = data.ptr();
	
	for (i = 0; i<h && count>0; i++, count--, list++, dataPtr += width)
	{
		const char *s = list[0];
		int l = strlen(s);
		int n = l/cpp;
		for (int j = 0; j<n; j++, s+=cpp) {
			int id = GetColorId(&tree, s, cpp);
//printf("%i,", id);
			dataPtr[j]=id;
		}
	}

	return true;
}

/////////////////////////// cicon //////////////////////////////////

static Mutex iconCopyMutex(true); 
static Mutex iconListMutex(true); 

static cinthash< int, ccollect< cptr< cicon > > > cmdIconList;

void cicon::SetCmdIcon(int cmd, const Image32 &image, int w, int h)
{
	MutexLock lock(&iconListMutex);
	ccollect< cptr<cicon> > &p = cmdIconList[cmd];
	int n = p.count();
	
	cptr<cicon> pIcon = new cicon();
	pIcon->Load(image, w, h);
	
	for( int i = 0; i<n; i++)
	{
		if (p[i]->Width() == w && p[i]->Height() == h)
		{
			p[i] = pIcon;
			return;
		}
	}
	
	cmdIconList[cmd].append(pIcon);
}

void cicon::SetCmdIcon(int cmd, const char **s, int w, int h)
{
	XPMImage im;
	im.Load(s, 200000);
	Image32 image32;
	image32.copy(im);
	cicon::SetCmdIcon(cmd, image32, w, h);
}


void cicon::ClearCmdIcons(int cmd)
{
	MutexLock lock(&iconListMutex);
	cmdIconList.del(cmd, false);
}

inline int IconDist(const cicon &a, int w, int h)
{
	w-=a.Width(); if (w<0) w=-w;
	h-=a.Height(); if (h<0) h=-h;
	return w+h;
}


void cicon::Load(int cmd, int w, int h)
{
	if (data) Clear();
	
	MutexLock lock(&iconListMutex);
	
	ccollect< cptr<cicon> > *p = cmdIconList.exist(cmd);
	if (!p) return;
	int n = p->count();
	if (n<=0) return;
	
	cptr<cicon> *t = p->ptr();
	
	cicon *minIcon = t[0].ptr();
	t++;
	
	int minDist = IconDist(*minIcon, w, h);
	
	
	if (!minDist) 
	{
		Copy(*minIcon);
		return;
	}
	
	for (int i = 1; i<n; i++, t++)
	{
		int dist = IconDist(t[i].ptr()[0], w, h);
		if (!dist) {
			Copy(t[i].ptr()[0]);
			return;
		}
		if (minDist>dist) {
			minIcon = t[i].ptr();
			minDist = dist;
		}
	}
	
	Load(minIcon->data->image, w, h);
}

void cicon::Load(const char **list, int w, int h)
{
	XPMImage xpm;
	xpm.Load(list, 100000);
	Image32 im;
	im.copy(xpm);
	Load(im, w, h);
}

void cicon::Copy(const cicon &a)
{
	if (data) Clear();
	if (!a.data) return;
	MutexLock lock(&iconCopyMutex);
	data = a.data;
	data->counter++;
}

void cicon::Load(const Image32 &image, int w, int h)
{
	if (data) Clear();
	
	IconData *p = new IconData;
	try {
		p->counter = 1;
//printf("load cicon %i, %i\n", w, h);		
		p->image.copy(image, w, h);
	} catch (...) {
		delete p;
		throw;
	}
	data = p;
}


void cicon::Clear()
{
	if (data) 
	{
		MutexLock lock(&iconCopyMutex);
		data->counter--;
		if (data->counter<=0) delete data;
		data = 0;
	}
}


inline unsigned Dis(unsigned a)
{
	unsigned b = ((a>>16)&0xFF);
	unsigned g = ((a>>8)&0xFF);
	unsigned r = ((a)&0xFF);
	unsigned m =  (r+g+b)/2 + 30;
	if (m>230) m = 230;
	return (a&0xFF000000) + (m<<16) + (m<<8) +m;
}

void MakeDisabledImage32(Image32 *dest, const Image32 &src)
{
	if (!dest) return;
	dest->copy(src);
	int n = dest->height() * dest->width();
	if (n<=0) return;
	unsigned32 *p = dest->line(0);
	for (;n>0; n--, p++)
		*p = Dis(*p);
}
}; //namespace wal

#include "swl_wincore_ui_inc.h"
