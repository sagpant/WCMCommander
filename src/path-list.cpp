/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#ifdef _WIN32
#  include <winsock2.h>
#endif

#include "path-list.h"
#include "vfs.h"
#include "vfs-ftp.h"
#include "vfs-sftp.h"
#include "vfs-smb.h"
#include "unicode_lc.h"


//TODO: move to common string utils class?
int PathList::Compare(const unicode_t* a, const unicode_t* b, bool ignoreCase)
{
    unicode_t au = 0;
    unicode_t bu = 0;

    for (; *a; a++, b++)
    {
        au = (ignoreCase ? UnicodeLC(*a) : *a);
        bu = (ignoreCase ? UnicodeLC(*b) : *b);

        if (au != bu)
        {
            break;
        }
    };

    return (*a ? (*b ? (au < bu ? -1 : (au == bu ? 0 : 1)) : 1) : (*b ? -1 : 0));
}

void PathList::GetStrings(std::vector<std::string>& list)
{
    for (int i = 0; i < m_list.count(); i++)
    {
        if (m_list[i].conf.ptr() && m_list[i].name.data())
        {
            m_list[i].conf->Set("name", unicode_to_utf8(m_list[i].name.data()).data());
            list.push_back(std::string(m_list[i].conf->GetConfig().data()));
        }
    }
}

void PathList::SetStrings(std::vector<std::string>& list)
{
    m_list.clear();

    for (int i = 0; i < (int)list.size(); i++)
    {
        if (list[i].data())
        {
            clPtr<StrConfig> cfg = new StrConfig();
            cfg->Load(list[i].data());
            const char* name = cfg->GetStrVal("name");

            if (name)
            {
                Data data;
                data.conf = cfg;
                data.name = utf8_to_unicode(name);
                m_list.append(data);
            }
        }
    }
}


PathListWin::PathListWin(Win* parent, PathList& dataList)
    : VListWin(Win::WT_CHILD, WH_TABFOCUS | WH_CLICKFOCUS, 0, parent, VListWin::SINGLE_SELECT, VListWin::BORDER_3D, 0),
    m_dataList(dataList)
{
    wal::GC gc(this);
    gc.Set(GetFont());

    cpoint ts = gc.GetTextExtents(ABCString);
    const int fontH = ts.y + 2;
    this->SetItemSize((fontH > 16 ? fontH : 16) + 1, 100);

    LSize ls;
    ls.x.maximal = 10000;
    ls.x.ideal = 1000;
    ls.x.minimal = 600;

    ls.y.maximal = 10000;
    ls.y.ideal = 600;
    ls.y.minimal = 400;

    SetLSize(ls);

    if (m_dataList.GetCount() > 0)
    {
        SetCount(m_dataList.GetCount());
        SetCurrent(0);
    }
}

void PathListWin::Sort()
{
    PathList::Data* curr = GetCurrentData();
    m_dataList.Sort();

    if (curr)
    {
        const unicode_t* sel = curr->name.data();

        for (int i = 0; i < m_dataList.GetCount(); i++)
        {
            if (!PathList::Compare(sel, m_dataList.GetData(i)->name.data(), true))
            {
                SetCurrent(i);
                Invalidate();
                break;
            }
        }
    }
}

static const int uiFcColor = GetUiID("first-char-color");

void PathListWin::DrawItem(wal::GC& gc, int n, crect rect)
{
    PathList::Data* curr = m_dataList.GetData(n);
    if (curr)
    {
        unicode_t* name = curr->name.data();

        UiCondList ucl;
        if ((n % 2) == 0)
        {
            ucl.Set(uiOdd, true);
        }

        if (n == this->GetCurrent())
        {
            ucl.Set(uiCurrentItem, true);
        }

        unsigned bg = UiGetColor(uiBackground, uiItem, &ucl, 0xB0B000);
        unsigned color = UiGetColor(uiColor, uiItem, &ucl, 0);
        unsigned fcColor = UiGetColor(uiFcColor, uiItem, &ucl, 0xFFFF);

        gc.SetFillColor(bg);
        gc.FillRect(rect);

        gc.Set(GetFont());
        gc.SetTextColor(color);
        gc.TextOutF(rect.left + 10, rect.top + 1, name);
        gc.SetTextColor(fcColor);
        gc.TextOutF(rect.left + 10, rect.top + 1, name, 1);
    }
    else
    {
        gc.SetFillColor(UiGetColor(uiBackground, 0, 0, 0xB0B000));
        gc.FillRect(rect);
    }
}


static const int uiClassShortcut = GetUiID("Shortcuts");
int PathListDlg::UiGetClassId() { return uiClassShortcut; }

void PathListDlg::OnSelected()
{
    PathList::Data* data = m_listWin.GetCurrentData();

    if (data && data->conf.ptr() && data->name.data())
    {
        m_selectedData = data;
        EndModal(CMD_OK);
    }
}

bool PathListDlg::Command(int id, int subId, Win* win, void* data)
{
    if (id == CMD_ITEM_CLICK && win == &m_listWin)
    {
        OnSelected();
        return true;
    }

    if (id == CMD_OK)
    {
        OnSelected();
        return true;
    }

    return NCDialog::Command(id, subId, win, data);
}

bool PathListDlg::OnKey(cevent_key* pEvent)
{
    if (pEvent->Type() == EV_KEYDOWN && pEvent->Key() == VK_RETURN && m_listWin.InFocus())
    {
        OnSelected();
        return true;
    }

    return false;
}


bool PathListDataToFS(PathList::Data* data, clPtr<FS>* fp, FSPath* pPath)
{
    StrConfig* cfg = data->conf.ptr();
    if (!cfg)
    {
        return false;
    }

    const char* type = cfg->GetStrVal("TYPE");
    const char* path = cfg->GetStrVal("PATH");

    if (!path)
    {
        return false;
    }

    if (!strcmp(type, "SYSTEM"))
    {
#ifdef _WIN32
        const int drive = cfg->GetIntVal("DISK");
        *fp = new FSSys(drive >= 0 ? drive : -1);
#else
        *fp = new FSSys();
#endif
        pPath->Set(CS_UTF8, path);
        return true;
    }

    if (!strcmp(type, "FTP"))
    {
        FSFtpParam param;
        param.SetConf(*cfg);
        *fp = new FSFtp(&param);
        pPath->Set(CS_UTF8, path);
        return true;
    }

#if defined(LIBSSH_EXIST) || defined(LIBSSH2_EXIST)
    if (!strcmp(type, "SFTP"))
    {
        FSSftpParam param;
        param.SetConf(*cfg);
        *fp = new FSSftp(&param);
        pPath->Set(CS_UTF8, path);
        return true;
    }
#endif

#ifdef LIBSMBCLIENT_EXIST
    if (!strcmp(type, "SMB"))
    {
        FSSmbParam param;
        param.SetConf(*cfg);
        *fp = new FSSmb(&param);
        pPath->Set(CS_UTF8, path);
        return true;
    }
#endif

    return false;
}

bool PathListFSToData(PathList::Data& data, clPtr<FS>* fs, FSPath* path)
{
    clPtr<StrConfig> cfg = new StrConfig();
    
    if (fs[0]->Type() == FS::SYSTEM)
    {
        cfg->Set("TYPE", "SYSTEM");
        cfg->Set("Path", path->GetUtf8());
#ifdef _WIN32
        const int disk = ((FSSys*)fs[0].Ptr())->Drive();
        if (disk >= 0)
        {
            cfg->Set("Disk", disk);
        }
#endif
    }
    else if (fs[0]->Type() == FS::FTP)
    {
        cfg->Set("TYPE", "FTP");
        cfg->Set("Path", path->GetUtf8());
        FSFtpParam param;
        ((FSFtp*)fs[0].Ptr())->GetParam(&param);
        param.GetConf(*cfg.ptr());
    }
    else

#if defined(LIBSSH_EXIST) || defined(LIBSSH2_EXIST)
        if (fs[0]->Type() == FS::SFTP)
        {
            cfg->Set("TYPE", "SFTP");
            cfg->Set("Path", path->GetUtf8());
            FSSftpParam param;
            ((FSSftp*)fs[0].Ptr())->GetParam(&param);
            param.GetConf(*cfg.ptr());
        }
        else
#endif

#ifdef LIBSMBCLIENT_EXIST
            if (fs[0]->Type() == FS::SAMBA)
            {
                cfg->Set("TYPE", "SMB");
                cfg->Set("Path", path->GetUtf8());
                FSSmbParam param;
                ((FSSmb*)fs[0].Ptr())->GetParam(&param);
                param.GetConf(*cfg.ptr());
            }
            else
#endif
                return false;

    data.name = utf8_to_unicode(fs[0]->Uri(*path).GetUtf8());
    data.conf = cfg;
    return true;
}
