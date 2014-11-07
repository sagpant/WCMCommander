/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#include "dlg-ctrl-l.h"
#include "ltext.h"

#define VALUE_WIDTH 12

static unicode_t* PrintableSizeStr(unicode_t buf[64], int64_t size)
{
	seek_t num = size;	

	char dig[64];
	char *end = unsigned_to_char<seek_t>(num, dig);
	int l = strlen(dig);

	unicode_t *us = buf;
	
	for (char *s = dig; l>0; s++, l--)
	{
		if ((l%3) == 0) *(us++) = ' ';
		*(us++) = *s;
	}
		
	*us = 0;
	
	return buf;
}


class DirCtrlL: public NCDialog {
	Layout lo;
	ccollect< clPtr<Win> > list;
	int row;
	void PutLabel(const char *s);
	void PutValue(const char *name, const char *value);
	void PutValue(const char *name, const unicode_t *value);
	void PutSpace(int height);
public:
	DirCtrlL(NCDialogParent *parent, FSStatVfs &statVfs)
	:	NCDialog(::createDialogAsChild, 0, parent, utf8_to_unicode(_LT("Information")).data(), bListOk),
		lo(50, 5),
		row(0)
	{
		//PutLabel( _LT("Information") );

		char hostName[0x100]="";
		gethostname(hostName, sizeof(hostName));
		PutValue( _LT("Computer name:"), hostName);

#ifdef _WIN32
	#define BUFLEN (0x100)
		wchar_t buf[BUFLEN];
		DWORD l = BUFLEN - 1;
		if (!GetUserNameW(buf, &l)) buf[0] = 0;
		unicode_t u[BUFLEN];
		int i;
		for (i = 0; i < BUFLEN - 1 && buf[i]; i++)
			u[i] = buf[i];
		u[i] = 0;
	#undef BUFLEN
		PutValue( _LT("User name:"), u);
#else
		{
			const char *user = getenv("USER");
			if (user) 
				PutValue( _LT("User name:"), user);
			
		}
#endif
		PutSpace(10);
		PutLabel( _LT("Disk") );
		{
			unicode_t buf[64];
			PutValue( _LT("Total bytes:"), PrintableSizeStr(buf, statVfs.size));
			PutValue( _LT("Free bytes:"), PrintableSizeStr(buf, statVfs.avail));
		}

		PutSpace(10);

#ifdef _WIN32 
		SYSTEM_INFO si;
		GetNativeSystemInfo(&si);
		PutLabel( _LT("System") );
		{
			const char *pt = "";
			switch (si.wProcessorArchitecture){
			case 0: pt = "x86"; break;
			case 6: pt = "IA64"; break;
			case 9: pt = "x64"; break;
			}
			char buf[0x100];
			sprintf(buf, "%s (%i)", pt, int(si.dwNumberOfProcessors));
			PutValue( _LT("Processor:"), buf);
		}

		MEMORYSTATUSEX ms;
		ms.dwLength = sizeof(ms);
		if (GlobalMemoryStatusEx(&ms))
		{
			PutSpace(5);
			unicode_t buf[64];
			PutValue( _LT("Total phisical memory:"), PrintableSizeStr(buf, ms.ullTotalPhys));
			PutValue( _LT("Free phisical memory:"), PrintableSizeStr(buf, ms.ullAvailPhys));
			PutSpace(5);
			PutValue( _LT("Total paging file:"), PrintableSizeStr(buf, ms.ullTotalPageFile));
			PutValue( _LT("Free paging file:"), PrintableSizeStr(buf, ms.ullAvailPageFile));
		}

#else
#endif

		lo.ColSet(0, 20);
		lo.ColSet(2, 10);
		lo.ColSet(4, 20);
		lo.SetColGrowth(4);
		lo.SetColGrowth(0);
		
		this->AddLayout(&lo);
		this->SetEnterCmd(CMD_OK);
		SetPosition();
	}

	bool Key(cevent_key* pEvent);
	virtual bool EventKey(cevent_key* pEvent);
	virtual bool EventChildKey(Win* child, cevent_key* pEvent);
	virtual ~DirCtrlL();
};

void DirCtrlL::PutLabel(const char *s)
{
	int n = row;
	clPtr<Win> win = new StaticLine(0, this, utf8_to_unicode( s ).data());
	lo.AddWin(win.ptr(), n, 0, n, 4, Layout::CENTER);
	win->Show(); win->Enable(); list.append(win);
	row++;
}

void DirCtrlL::PutValue(const char *name, const char *value)
{
	int n = row;
	clPtr<Win> win = new StaticLine(uiVariable, this, utf8_to_unicode( name ).data());
	lo.AddWin(win.ptr(), n, 1);
	win->Show(); win->Enable(); list.append(win);

	win = new StaticLine(uiValue, this, utf8_to_unicode( value ).data());
	lo.AddWin(win.ptr(), n, 3, n, 3, Layout::RIGHT);
	win->Show(); win->Enable(); list.append(win);
	row++;
}

void DirCtrlL::PutValue(const char *name, const unicode_t *value)
{
	int n = row;
	clPtr<Win> win = new StaticLine(uiVariable, this, utf8_to_unicode( name ).data());
	lo.AddWin(win.ptr(), n, 1);
	win->Show(); win->Enable(); list.append(win);

	win = new StaticLine(uiValue, this,  value );
	lo.AddWin(win.ptr(), n, 3, n, 3, Layout::RIGHT);
	win->Show(); win->Enable(); list.append(win);
	row++;
}

void DirCtrlL::PutSpace(int height)
{
	lo.LineSet(row, height);
	row++;
}

bool DirCtrlL::Key(cevent_key* pEvent)
{
	if (pEvent->Type() == EV_KEYDOWN) 
	{
		 if (pEvent->Key() == VK_L && pEvent->Mod() == KM_CTRL )
		 {
			EndModal(CMD_OK);
			return true;
		 }
	}

	return false;
}


bool DirCtrlL::EventKey(cevent_key* pEvent)
{
	if (Key(pEvent)) return true;
	return NCDialog::EventKey(pEvent);
}

bool DirCtrlL::EventChildKey(Win* child, cevent_key* pEvent)
{
	if (Key(pEvent)) return true;
	return NCDialog::EventChildKey(child, pEvent);
}

DirCtrlL::~DirCtrlL()
{
}

void DoCtrlLDialog(NCDialogParent *parent, FSStatVfs statVfs)
{
	DirCtrlL dlg(parent, statVfs);
	dlg.Enable();
	dlg.Show();
	int cmd = dlg.DoModal();
}
