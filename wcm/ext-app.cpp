#include <wal.h>

#ifdef _WIN32

#include "ext-app.h"
#include "string-util.h"
#include "w32util.h"

static carray<wchar_t> GetOpenApp(wchar_t *ext)
{
	if (!ext) return carray<wchar_t>();
	RegKey key;
	if (!key.Open(HKEY_CURRENT_USER, carray_cat<wchar_t>(L"software\\classes\\", ext).ptr()))
		key.Open(HKEY_CLASSES_ROOT, ext);

	carray<wchar_t> p = key.GetString();

	if (!key.Open(HKEY_CURRENT_USER, carray_cat<wchar_t>(L"software\\classes\\", p.ptr(),L"\\shell\\open\\command").ptr()))
		key.Open(HKEY_CLASSES_ROOT, carray_cat<wchar_t>(p.ptr(),L"\\shell\\open\\command").ptr());

	return key.GetString();
}

static carray<unicode_t> CfgStringToCommand(const wchar_t *cfgCmd, const unicode_t *uri)
{
	ccollect<unicode_t, 0x100> res;
	int insCount = 0;
	
	int prev = 0;
	for (const wchar_t *s = cfgCmd; *s; prev = *s, s++)
	{
		if (*s == '%' && (s[1] == '1' || s[1]=='l' || s[1] == 'L'))
		{
			if (prev != '"' && prev !='\'') res.append('"');
			for (const unicode_t *t = uri; *t; t++) res.append(*t);
			if (prev != '"' && prev !='\'') res.append('"');
			s++;
			insCount++;
		} else res.append(*s);
	}

	if (!insCount)
	{
		res.append(' ');
		res.append('"');
		for (const unicode_t *t = uri; *t; t++) res.append(*t);
		res.append('"');
	}

	res.append(0);

	return res.grab();
}

static carray<wchar_t> GetFileExt(const unicode_t *uri)
{
	if (!uri) return carray<wchar_t>();
	const unicode_t *ext = find_right_char<unicode_t>(uri,'.');
	if (!ext || !*ext) return carray<wchar_t>();
	return UnicodeToUtf16(ext);
}

static carray<unicode_t> NormalizeStr(unicode_t*s)
{
	if (!s) return 0;
	int n = unicode_strlen(s);
	carray<unicode_t> p(n+1);
	unicode_t *t = p.ptr();
	for (; *s; s++) if (*s!='&') *(t++) = *s;
	*t = 0;
	return p;
}

cptr<AppList> GetAppList(const unicode_t *uri)
{
	carray<wchar_t> ext = GetFileExt(uri);
	if (!ext.ptr()) return 0;
	RegKey key;
	if (!key.Open(HKEY_CURRENT_USER, carray_cat<wchar_t>(L"software\\classes\\", ext.ptr()).ptr()))
		key.Open(HKEY_CLASSES_ROOT, ext.ptr());

	carray<wchar_t> p = key.GetString();

	RegKey key2;

	if (!key2.Open(HKEY_CURRENT_USER, carray_cat<wchar_t>(L"software\\classes\\", p.ptr(),L"\\shell").ptr()))
		key2.Open(HKEY_CLASSES_ROOT, carray_cat<wchar_t>(p.ptr(),L"\\shell").ptr());


	if (!key2.Ok()) return 0;

	cptr<AppList> ret = new AppList();

	carray<wchar_t> pref = key2.GetString();

	for (int i = 0; i<10; i++) 
	{
		carray<wchar_t> sub = key2.SubKey(i);
		if (!sub.ptr()) break;
	
		RegKey key25;
		key25.Open(key2.Key(), sub.ptr());
		if (!key25.Ok()) continue;

		carray<wchar_t> name = key25.GetString();
//wprintf(L"%s, %s\n", sub.ptr(), name.ptr());
		RegKey key3;
		key3.Open(key25.Key(), L"command");
		carray<wchar_t> command = key3.GetString();

		if (command.ptr())
		{
			AppList::Node node;
			node.name = NormalizeStr(Utf16ToUnicode(name.ptr() && name[0]? name.ptr() : sub.ptr()).ptr());
			node.cmd = CfgStringToCommand(command.ptr(), uri);

			if ( (pref.ptr() && !wcsicmp(pref.ptr(), sub.ptr()) || !pref.ptr() && !wcsicmp(L"Open", sub.ptr())) && ret->list.count()>0)
			{
				ret->list.insert(0);
				ret->list[0] = node;
			} else
				ret->list.append(node);
		}
	}
	
	key2.Open(key.Key(), L"OpenWithList");
	if (key2.Ok())
	{
		cptr<AppList> openWith = new AppList();

		for (int i =0; i<10; i++) {
			carray<wchar_t> sub = key2.SubKey(i);
			if (!sub.ptr()) break;

			RegKey keyApplication;
			keyApplication.Open(HKEY_CLASSES_ROOT, 
				carray_cat<wchar_t>(L"Applications\\", sub.ptr(), L"\\shell\\open\\command").ptr());
			carray<wchar_t> command = keyApplication.GetString();
			if (command.ptr())
			{
				AppList::Node node;
				node.name = NormalizeStr(Utf16ToUnicode(sub.ptr()).ptr());
				node.cmd = CfgStringToCommand(command.ptr(), uri);
				openWith->list.append(node);
			}
		}
		if (openWith->Count()>0) {
			AppList::Node node;
			static unicode_t openWidthString[]={ 'O','p','e','n',' ','w','i','t','h',0};
			node.name = new_unicode_str(openWidthString);
			node.sub = openWith;
			ret->list.append(node);
		}
	}

	return ret->Count() ? ret : 0;
}


carray<unicode_t> GetOpenCommand(const unicode_t *uri, bool *needTerminal, const unicode_t **pAppName)
{
	carray<wchar_t> wCmd = GetOpenApp(GetFileExt(uri).ptr());
	if (!wCmd.ptr()) return carray<unicode_t>();
	return CfgStringToCommand(wCmd.ptr(), uri);
}


#endif
