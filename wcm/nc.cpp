/*
	Copyright (c) by Valery Goryachev (Wal)
*/
#ifdef _WIN32
#include <Winsock2.h>
#include "resource.h"
#include "w32util.h"
#endif

#include "nc.h"
#include "ncfonts.h"
#include "ncwin.h"
#include "panel.h"

#include <signal.h>
#include "ext-app.h"
#include "wcm-config.h"
#include "color-style.h"
#include "vfs-sftp.h"
#include "charsetdlg.h"
#include "string-util.h"
#include "ltext.h"
#include "globals.h"

#include "intarnal_icons.inc"

cptr<wal::GC> defaultGC;

const char *appName = "Wal Commander";


unsigned (*OldSysGetColor)(Win *w, int id)=0;
unsigned MSysGetColor(Win *w, int id)
{
	switch (id) {
	case IC_MENUBOX_BG: return ::wcmConfig.whiteStyle ? 0xD8E9EC :0xB0B000;
	case IC_MENUBOX_BORDER: return 0xB0B000;
	case IC_MENUBOX_TEXT:   return 0x01;
	case IC_MENUBOX_SELECT_BG: return 0x01;
	case IC_MENUBOX_SELECT_TEXT: return 0xFFFFFF;
	case IC_MENUBOX_SELECT_FRAME: return 0x00FF00;

	case IC_MENUPOPUP_BG: return ::wcmConfig.whiteStyle ? /*0xD8E9EC*/0xFFFFFF : 0xA0A000;
	case IC_MENUPOPUP_LEFT: return ::wcmConfig.whiteStyle ? 0xD8E9EC : 0xC0C000;	
	case IC_MENUPOPUP_BORDER: return ::wcmConfig.whiteStyle ? 0x808080 : 0xFFFFFF;
	case IC_MENUPOPUP_LINE:	return 0;
	case IC_MENUPOPUP_TEXT:	return ::wcmConfig.whiteStyle ? 0 :0xFFFFFF; //0;
	case IC_MENUPOPUP_SELECT_TEXT: return ::wcmConfig.whiteStyle ? 0 :0xFFFFFF;
	case IC_MENUPOPUP_SELECTBG: return ::wcmConfig.whiteStyle ? 0xB0C0C0 : 0x404000;
	case IC_MENUPOPUP_POINTER: return 0;
	
	case IC_SCROLL_BORDER: return ::wcmConfig.whiteStyle ? 0xD8E9EC : 0x606000; //0xFFD0D0;
	case IC_SCROLL_BG: return ::wcmConfig.whiteStyle ? 0xE8F9FC :0xA0A000; //0xA00000; //0xD0D000;
	case IC_SCROLL_BUTTON: return ::wcmConfig.whiteStyle ? 0xD8E9EC : 0xFFFF00;
	};

	if (w && w->Parent() && w->Parent()->GetClassId() == CI_BUTTON_WIN)
	{
		switch(id) {
		case IC_BG: return ::wcmConfig.whiteStyle ? 0xD8E9EC : 0xB0B000;
		}
	}
	
	if (id==IC_EDIT_TEXT_BG) return ::wcmConfig.whiteStyle ? 0xFFFFFF : 1;
	if (id==IC_EDIT_TEXT) return ::wcmConfig.whiteStyle ? 0 : 0xFFFFFF;
	
	if (w &&  w->GetClassId() == CI_TOOLTIP)
	{
		switch(id) {
		case IC_BG: return 0x80FFFF;
		case IC_TEXT: return 0;
		}
	}

	return OldSysGetColor(w,id);
}

cfont* (*OldSysGetFont)(Win *w, int id)=0;
cfont* MSysGetFont(Win *w, int id)
{
	if (w) {
		if (w->GetClassId() ==  CI_PANEL)
			return panelFont.ptr();

		if (w->GetClassId() == CI_VLIST)
			return  panelFont.ptr();
			
		if (w->GetClassId() == CI_TERMINAL)
			return  terminalFont.ptr();
			
		if (w->GetClassId() == CI_EDITOR)
			return  editorFont.ptr();
			
		if (w->GetClassId() == CI_VIEWER)
			return  viewerFont.ptr();

		if (	w->GetClassId() ==  CI_MENUBAR || 
			w->GetClassId() ==  CI_POPUPMENU || 
			w->GetClassId() ==  CI_TOOLTIP ||
//			w->GetClassId() ==  CI_EDITORHEADWIN || 
			w->Parent() && w->GetClassId() == CI_BUTTON)
		{
			return dialogFont.ptr();
		}
	}
	return editorFont.ptr();
}

#ifdef _WIN32
namespace wal {
extern int Win32HIconId;
extern HINSTANCE Win32HInstance;
};
#endif

inline carray<sys_char_t> LTPath(const sys_char_t *fn, const char *ext)
{
	return carray_cat<sys_char_t>(fn, utf8_to_sys(ext).ptr());
}

static  bool InitLocale(const sys_char_t *dir, const char *id)
{
	if (id[0]=='-') return true;
	carray<sys_char_t> fn = 
#ifdef _WIN32
			carray_cat<sys_char_t>(dir, utf8_to_sys("\\ltext.").ptr());
#else
			carray_cat<sys_char_t>(dir, utf8_to_sys("/ltext.").ptr());
#endif

	if (id[0]!='+') 
		return LTextLoad( LTPath(fn.ptr(), id).ptr() );
	
	return LTextLoad( LTPath(fn.ptr(), sys_locale_lang_ter()).ptr()) ||
		LTextLoad( LTPath(fn.ptr(), sys_locale_lang() ).ptr());
};

#ifdef _WIN32
int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR    lpCmdLine, int       nCmdShow)
#else 
int main(int argc, char **argv)
#endif
{	
	try {

		#ifndef _WIN32
			for (int i = 1; i<argc; i++)
			{
				if (argv[i][0]=='-' && argv[i][1]=='-' && !strcmp(argv[i]+2, "dlg"))
					createDialogAsChild = false;
			}
		#endif
	
		#ifdef _WIN32 
		//disable system critical messageboxes (for example: no floppy disk :) )
		SetErrorMode(SEM_FAILCRITICALERRORS);
		#endif

		#ifdef _WIN32
		Win32HInstance = hInstance;
		Win32HIconId = IDI_WCM;
		#endif


#ifdef _WIN32
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD( 2, 2 ), &wsaData )) throw_syserr(0,"WSAStartup failed");
#else
		setlocale(LC_ALL, "");
		signal(SIGPIPE, SIG_IGN);
#endif
		
		try {
			InitConfigPath();
			wcmConfig.Load();
			
			const char *langId = wcmConfig.systemLang.ptr() ? wcmConfig.systemLang.ptr() : "+";
#ifdef _WIN32
			InitLocale(carray_cat<sys_char_t>(GetAppPath().ptr(), utf8_to_sys("lang").ptr() ).ptr(), langId);
#else
			if (!InitLocale("install-files/share/wcm/lang", langId))
				InitLocale(UNIX_CONFIG_DIR_PATH "/lang", langId);
#endif

			SetPanelColorStyle(wcmConfig.panelColorMode);
			SetEditorColorStyle(wcmConfig.editColorMode);
			SetViewerColorStyle(wcmConfig.viewColorMode);
		} catch (cexception *ex) {
			fprintf(stderr, "%s\n", ex->message());
			ex->destroy();
		}
		
		InitSSH();

		AppInit();
		OldSysGetColor =SysGetColor;
		SysGetColor=MSysGetColor;

		OldSysGetFont =SysGetFont;
		SysGetFont=MSysGetFont;


		cfont *defaultFont = SysGetFont(0,0);
		defaultGC = new wal::GC((Win*)0);
		
#ifndef _WIN32
#include "icons/wcm.xpm"
		Win::SetIcon(wcm_xpm);
#endif		

		InitFonts();
		InitOperCharsets();

		SetCmdIcons();

		NCWin ncWin;

		ncWin.Enable();	ncWin.Show();

		InitExtensionApp();		
				
		AppRun();

		dbg_printf("App Quit!!!");

	} catch (cexception *ex)
	{
	#ifdef _WIN32
		MessageBoxA(0, ex->message(), "", MB_OK);
	#else 
		printf("Error: %s\n", ex->message());
	#endif
	}
	return 0;
}
