/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#pragma once

#include <algorithm>

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

        bool operator <= (const Data& a) const
        {
            return (PathList::Compare(name.data(), a.name.data(), true) <= 0);
        }
		  bool operator < (const Data& a) const
		  {
			  return (PathList::Compare(name.data(), a.name.data(), true) < 0);
		  }
    };

    static int Compare(const unicode_t* a, const unicode_t* b, bool ignoreCase);

private:
    std::vector<Data> m_list;
    
public:
    void GetStrings(std::vector<std::string>& list) const;

    void SetStrings(const std::vector<std::string>& list);

    const PathList::Data* GetData(const int i) const
    {
        if (i < 0 || i >= (int)m_list.size())
        {
            return nullptr;
        }

        return &(m_list[i]);
    }

    int GetCount() const
    {
        return (int)m_list.size();
    }

    // Returns index of element with the specified name, or -1 if no such found
    int FindByName(const unicode_t* name) const
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

        m_list.push_back(data);
        return true;
    }

    bool Remove(const int i)
    {
        if (i < 0 || i >= (int)m_list.size())
        {
            return false;
        }

		  m_list.erase( m_list.begin( ) + i );
        return true;
    }

    bool Rename(const int i, const unicode_t* name)
    {
        if (!name || i < 0 || i >= (int)m_list.size())
        {
            return false;
        }

        m_list[i].name = new_unicode_str(name);
        return true;
    }

    void Sort()
    {
        if ( !m_list.empty() )
        {
				std::sort( m_list.begin(), m_list.end(), std::less<Data>() );
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
    const PathList::Data* GetCurrentData() const
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
	virtual std::vector<unicode_t> GetItemText( const PathList::Data* Data ) const;
    virtual void OnItemListChanged()
    {
        CalcScroll();
        Invalidate();
    }
};


class PathListDlg : public NCDialog
{
protected:
    PathListWin&          m_ListWin;
    const PathList::Data* m_SelectedData;

public:
    PathListDlg( NCDialogParent* Parent, PathListWin& ListWin, const char* szTitle, ButtonDataNode* blist )
     : NCDialog(::createDialogAsChild, 0, Parent, utf8_to_unicode(szTitle).data(), blist)
	  , m_ListWin( ListWin )
	  , m_SelectedData(0)
    {
        SetEnterCmd(CMD_OK);
    }

    virtual ~PathListDlg() {}

    const PathList::Data* GetSelected() const { return m_SelectedData; }

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


bool PathListDataToFS(const PathList::Data* data, clPtr<FS>* fp, FSPath* pPath);

bool PathListFSToData(PathList::Data& data, clPtr<FS>* fs, FSPath* path);
