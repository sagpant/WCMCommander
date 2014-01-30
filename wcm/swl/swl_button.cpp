/*
	Copyright (c) by Valery Goryachev (Wal)
*/


#include "swl.h"

namespace wal {

#define LEFTSPACE 5
#define RIGHTSPACE 5
#define ICONX_RIGHTSPACE 1

int Button::GetClassId()
{
	return CI_BUTTON;
}

void Button::OnChangeStyles()
{
	GC gc(this);
	gc.Set(GetFont());
	cpoint p = gc.GetTextExtents(text.ptr());

	if (icon.ptr()) 
	{
		if (p.y < 12)
			p.y = 12;
				
		p.x += ICONX_RIGHTSPACE + icon->Width();
		if (icon->Width() > p.y+4) 
			p.y = icon->Width()-4;
	}

	p.x += LEFTSPACE + RIGHTSPACE;
	p.x += 8;
	p.y += 8;
	SetLSize(LSize(p));
}

static unicode_t spaceUnicodeStr[]={' ', 0};

Button::Button(Win *parent, const unicode_t *txt, int id, crect *rect, int iconX, int iconY)
:	Win(Win::WT_CHILD,Win::WH_TABFOCUS|WH_CLICKFOCUS,parent, rect),
	pressed(false), 
	icon(new cicon(id, iconX, iconY)),
	commandId(id)
{
	if (!icon->Valid()) 
		icon.clear();
               
	text = new_unicode_str(txt && txt[0] ? txt : spaceUnicodeStr);
	
	if (!rect)
	{
		OnChangeStyles();
	}
};

void Button::Set(const unicode_t *txt, int id, int iconX, int iconY)
{
	text = new_unicode_str(txt && txt[0] ? txt : spaceUnicodeStr);
	
	commandId = id;
	icon = new cicon(id, iconX, iconY);
	if (!icon->Valid())
		icon.clear();
}

bool Button::EventMouse(cevent_mouse* pEvent)
{
	switch (pEvent->Type())	{
	case EV_MOUSE_MOVE:
		if (IsCaptured()) 
		{
			crect r = ClientRect();
			cpoint p = pEvent->Point();
			if (pressed)
			{
				if (p.x<0 || p.y<0 || p.x>=r.right || p.y>=r.bottom) 
				{
					pressed = false;
					Invalidate();
				}
			} else {
				if (p.x>=0 && p.y>=0 && p.x<r.right && p.y<r.bottom) 
				{
					pressed = true;
					Invalidate();
				}
			}
		}
		break;

	case EV_MOUSE_PRESS:
	case EV_MOUSE_DOUBLE:
		if (pEvent->Button()!=MB_L) break;
		SetCapture();
		pressed = true;
		Invalidate();
		break;

	case EV_MOUSE_RELEASE:
		if (pEvent->Button()!=MB_L) break;
		ReleaseCapture();
		if (pressed) SendCommand();
		pressed = false;
		Invalidate();
		break;
	};
	return true;
}

bool Button::EventFocus(bool recv)
{
	bool ret = Win::EventFocus(recv);
	if (!recv && pressed)
		pressed=false;
	Invalidate();
	return ret;
}

bool Button::EventKey(cevent_key* pEvent)
{
	if (pEvent->Type() == EV_KEYDOWN && pEvent->Key() == VK_RETURN)
	{
		pressed = true;
		Invalidate();
		return true;
	}
	if (pressed && pEvent->Type() == EV_KEYUP && pEvent->Key() == VK_RETURN)
	{
		pressed = false;
		Invalidate();
		SendCommand();
		return true;
	}
	return false;
}


void Button::Paint(GC &gc, const crect &paintRect)
{
	unsigned colorBg = GetColor(0);
	crect cr = this->ClientRect();
	crect rect = cr;
	DrawBorder(gc, rect, ColorTone(colorBg, +20));
	rect.Dec();
	DrawBorder(gc, rect, ColorTone(colorBg, -200));
	rect.Dec();

	if (pressed)
	{
		Draw3DButtonW2(gc, rect, colorBg, false);
		rect.Dec();
		rect.Dec();
	} else {
		Draw3DButtonW2(gc, rect, colorBg, true);
		rect.Dec();
		if (InFocus()) {
			DrawBorder(gc, rect, GetColor(IC_FOCUS_MARK)); 
		}
		rect.Dec();
	}

	gc.SetFillColor(colorBg);
	gc.FillRect(rect);
	gc.SetTextColor(GetColor(IsEnabled() ? IC_TEXT : IC_GRAY_TEXT));
	gc.Set(GetFont());
	cpoint tsize = gc.GetTextExtents(text.ptr());

	/*
	int l = tsize.x + (icon.ptr() ? icon->Width() + ICONX_RIGHTSPACE : 0);
	
	int w = rect.Width() - LEFTSPACE - RIGHTSPACE;
	
	if (icon.ptr()) w-=ICONX_RIGHTSPACE;
	
	//int x = rect.left + LEFTSPACE + (w > l ? (w - l)/2 : 0) +(pressed?2:0);
	int x = rect.left + LEFTSPACE + (w-l)/2 +(pressed?2:0);
	*/
	
	int l = tsize.x + (icon.ptr() ? icon->Width() + ICONX_RIGHTSPACE : 0);
	int w = rect.Width();
	int x = rect.left + (w > l ? (w - l)/2 : 0) +(pressed?2:0);

	
	if (icon.ptr())
	{
		gc.DrawIcon(x, rect.top + (rect.Height() - icon->Height())/2 + (pressed?2:0), icon.ptr());
		x += icon->Width() + ICONX_RIGHTSPACE;
	} 
	gc.SetClipRgn(&rect);
	gc.TextOutF(x, rect.top + (rect.Height()-tsize.y)/2+(pressed?2:0),text.ptr());
}

Button::~Button(){}

}; //namespace wal
