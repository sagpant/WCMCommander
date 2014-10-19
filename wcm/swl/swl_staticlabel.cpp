#include "swl.h"
// XXX: refactor to move the header to .
#include "../unicode_lc.h" 

namespace wal
{
	
	int uiClassStaticLabel = GetUiID("StaticLabel");

	int StaticLabel::UiGetClassId()
	{
		return uiClassStaticLabel;
	}

	StaticLabel::StaticLabel(int nId, Win* parent, const unicode_t* txt, Win* _master, crect* rect)
		: Win(Win::WT_CHILD, 0, parent, rect, nId), text(txt), master(_master)
	{
		if (!rect)
		{
			GC gc(this);
			SetLSize(LSize(text.GetTextExtents(gc, GetFont())));
		}
	}

	void StaticLabel::Paint(GC& gc, const crect& paintRect)
	{
		crect rect = ClientRect();
		gc.SetFillColor(UiGetColor(uiBackground, uiClassStaticLabel, 0, 0xFFFFFF)/*GetColor(0)*/);
		gc.FillRect(rect); 
		gc.Set(GetFont());
		text.DrawItem(gc, 0, 0, 
			UiGetColor(uiColor, uiClassStaticLabel, 0, 0), 
			UiGetColor(uiHotkeyColor, uiClassStaticLabel, 0, 0));
	}

	Win* StaticLabel::IsHisHotKey(cevent_key* pEvent)
	{
		return text.isHotkeyMatching(UnicodeUC(pEvent->Char())) ? master : 0;
	}

}

