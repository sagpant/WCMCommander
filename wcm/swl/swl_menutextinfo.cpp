#include "swl.h"
// XXX: refactor to move the header to .
#include "../unicode_lc.h" 

namespace wal
{
	void MenuTextInfo::ParseHkText(const unicode_t* inStr)
	{
		hotkeyUpperCase = 0;
		if (inStr == 0)
		{
			strBeforeHk = 0;
			strHk = 0;
			strAfterHk = 0;
			strFull = 0;
			return;
		}
		unicode_t* pAmpersand = unicode_strchr(inStr, '&');

		if (pAmpersand == 0 || pAmpersand[1] == 0) // no hot key defined, or HK is the last char
		{
			strBeforeHk = unicode_strdup(inStr);
			strHk = 0;
			strAfterHk = 0;

		}
		else // HK is defined, and is not the last char
		{
			if (pAmpersand == inStr) // hot key is 1st char
			{
				strBeforeHk = 0;
			}
			else // hot key exists, and is not the first char
			{
				int strlen_beforeHk = pAmpersand - inStr;
				strBeforeHk = new unicode_t[strlen_beforeHk + 1];
				unicode_strncpy0(strBeforeHk, inStr, strlen_beforeHk);
			}
			unicode_t hotKeyChar = pAmpersand[1];
			hotkeyUpperCase = UnicodeUC(hotKeyChar);
			strHk = new unicode_t[2];
			strHk[0] = hotKeyChar;
			strHk[1] = 0;
			if (pAmpersand[2] == 0)
			{
				strAfterHk = 0;
			}
			else
			{
				strAfterHk = unicode_strdup(pAmpersand + 2);
			}
		}
		// build strFull
		strFull = new unicode_t[unicode_strlen(inStr) + 1];
		strFull[0] = 0;
		if (strBeforeHk != 0)
			unicode_strcat(strFull, strBeforeHk);
		if (strHk != 0)
			unicode_strcat(strFull, strHk);
		if (strAfterHk != 0)
			unicode_strcat(strFull, strAfterHk);
	}

	MenuTextInfo::MenuTextInfo(const unicode_t* inStr)
	{
		ParseHkText(inStr);
	}

	MenuTextInfo::MenuTextInfo(const MenuTextInfo& src)
	{
		strBeforeHk = (src.strBeforeHk == 0) ? 0 : unicode_strdup(src.strBeforeHk);
		strHk       = (src.strHk       == 0) ? 0 : unicode_strdup(src.strHk);
		strAfterHk  = (src.strAfterHk  == 0) ? 0 : unicode_strdup(src.strAfterHk);
		strFull     = (src.strFull     == 0) ? 0 : unicode_strdup(src.strFull);
		hotkeyUpperCase = src.hotkeyUpperCase;
	}

	inline static void _clearUnicodeArray(unicode_t** p)
	{
		if (*p)
		{
			delete[] * p;
			*p = 0;
		}
	}
	void MenuTextInfo::Clear()
	{
		_clearUnicodeArray(&strBeforeHk);
		_clearUnicodeArray(&strHk);
		_clearUnicodeArray(&strAfterHk);
		_clearUnicodeArray(&strFull);
	}

	MenuTextInfo::~MenuTextInfo()
	{
		Clear();
	}

	void MenuTextInfo::SetText(const unicode_t* inStr)
	{
		Clear();
		ParseHkText(inStr);
	}

	void MenuTextInfo::DrawItem(GC& gc, int x, int y, int color_text, int color_hotkey) const
	{
		if (strBeforeHk != 0)
		{
			gc.SetTextColor(color_text);
			gc.TextOut(x, y, strBeforeHk);
			x += gc.GetTextExtents(strBeforeHk).x;
		}
		if (strHk != 0)
		{
			gc.SetTextColor(color_hotkey);
			gc.TextOut(x, y, strHk);
			x += gc.GetTextExtents(strHk).x;
		}
		if (strAfterHk != 0)
		{
			gc.SetTextColor(color_text);
			gc.TextOut(x, y, strAfterHk);
		}
	}

}