/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#pragma once

#include "ncdialogs.h"
#include "vfs.h"


class PathList
{
public:
    PathList() {}
    virtual ~PathList() {}

    struct Data
    {
        std::vector<unicode_t> name;
        clPtr<StrConfig> conf;

        bool operator <= (const Data& a)
        {
            return (PathList::Compare(name.data(), a.name.data(), true) <= 0);
        }
    };

    static int Compare(const unicode_t* a, const unicode_t* b, bool ignoreCase);

private:
    ccollect<Data> m_list;
    
public:
    void GetStrings(std::vector<std::string>& list);

    void SetStrings(std::vector<std::string>& list);

    PathList::Data* GetData(const int i)
    {
        if (i < 0 || i >= m_list.count())
        {
            return NULL;
        }

        return &(m_list[i]);
    }

    int GetCount()
    {
        return m_list.count();
    }

    // Returns index of element with the specified name, or -1 if no such found
    int FindByName(const unicode_t* name)
    {
        for (int i = 0; i < GetCount(); i++)
        {
            if (!PathList::Compare(name, GetData(i)->name.data(), false))
            {
                return i;
            }
        }
        
        return -1;
    }

    bool Add(const unicode_t* name, clPtr<StrConfig> p)
    {
        if (!name || !*name)
        {
            return false;
        }

        Data data;
        data.name = new_unicode_str(name);
        data.conf = p;

        m_list.append(data);
        return true;
    }

    bool Remove(const int i)
    {
        if (i < 0 || i >= m_list.count())
        {
            return false;
        }

        m_list.del(i);
        return true;
    }

    bool Rename(const int i, const unicode_t* name)
    {
        if (!name || i < 0 || i >= m_list.count())
        {
            return false;
        }

        m_list[i].name = new_unicode_str(name);
        return true;
    }

    void Sort()
    {
        const int count = m_list.count();
        if (count > 0)
        {
            sort2m<Data>(m_list.ptr(), count);
        }
    }
};


class PathListWin : public VListWin
{
protected:
    PathList&  m_dataList;

public:
    PathListWin(Win* parent, PathList& dataList);

    virtual ~PathListWin() {}

public:
    PathList::Data* GetCurrentData()
    {
        return m_dataList.GetData(GetCurrent());
    }

    void InsertItem(const unicode_t* name, clPtr<StrConfig> p)
    {
        if (m_dataList.Add(name, p))
        {
            SetCount(m_dataList.GetCount());
            SetCurrent(m_dataList.GetCount() - 1);

            OnItemListChanged();
        }
    }

    void DeleteCurrentItem()
    {
        if (m_dataList.Remove(GetCurrent()))
        {
            SetCount(m_dataList.GetCount());

            OnItemListChanged();
        }
    }

    void RenameCurrentItem(const unicode_t* name)
    {
        if (m_dataList.Rename(GetCurrent(), name))
        {
            OnItemListChanged();
        }
    }

    virtual void DrawItem(wal::GC& gc, int n, crect rect);

    void Sort();

protected:
    virtual void OnItemListChanged()
    {
        CalcScroll();
        Invalidate();
    }
};


class PathListDlg : public NCDialog
{
protected:
    PathListWin&       m_listWin;
    PathList::Data*    m_selectedData;

public:
    PathListDlg(NCDialogParent* parent, PathListWin& listWin, const char* szTitle, ButtonDataNode* blist)
        : NCDialog(::createDialogAsChild, 0, parent, utf8_to_unicode(szTitle).data(), blist),
        m_listWin(listWin),
        m_selectedData(0)
    {
        SetEnterCmd(CMD_OK);
    }

    virtual ~PathListDlg() {}

    PathList::Data* GetSelected() { return m_selectedData; }

    virtual bool Command(int id, int subId, Win* win, void* data);

    virtual int UiGetClassId();

    virtual bool EventChildKey(Win* child, cevent_key* pEvent)
    {
        if (OnKey(pEvent))
        {
            return true;
        }

        return NCDialog::EventChildKey(child, pEvent);
    }

    virtual bool EventKey(cevent_key* pEvent)
    {
        if (OnKey(pEvent))
        {
            return true;
        }

        return NCDialog::EventKey(pEvent);
    }

protected:
    virtual bool OnKey(cevent_key* pEvent);

    virtual void OnSelected();
};


bool PathListDataToFS(PathList::Data* data, clPtr<FS>* fp, FSPath* pPath);

bool PathListFSToData(PathList::Data& data, clPtr<FS>* fs, FSPath* path);
