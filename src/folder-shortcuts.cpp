/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * wcm@linderdaum.com
 */

#include "folder-shortcuts.h"
#include "path-list.h"
#include "string-util.h"
#include "wcm-config.h"
#include "ltext.h"
#include "unicode_lc.h"


static const char shortcursSection[] = "shortcuts";


class FolderShortcutsWin : public PathListWin
{
public:
	FolderShortcutsWin( Win* parent, PathList& dataList )
		: PathListWin( parent, dataList )
	{
		Load();
	}

	virtual ~FolderShortcutsWin() {}

private:
	void Load()
	{
		std::vector<std::string> list;
		LoadStringList( shortcursSection, list );

		m_dataList.SetStrings( list );
		Sort();

		SetCount( m_dataList.GetCount() );
		SetCurrent( 0 );
	}

	void Save()
	{
		std::vector<std::string> list;
		m_dataList.GetStrings( list );

		SaveStringList( shortcursSection, list );
	}

protected:
	virtual void OnItemListChanged()
	{
		Save();
		Sort();

		PathListWin::OnItemListChanged();
	}
};


#define CMD_PLUS    1000
#define CMD_MINUS   1001
#define CMD_RENAME  1002

ButtonDataNode buttonsData[] =
{
	{ "&Insert", CMD_PLUS },
	{ "&Delete", CMD_MINUS },
	{ "&Rename", CMD_RENAME },
	{ 0, 0 }
};

class FolderShortcutsDlg : public PathListDlg
{
private:
	PathList                m_dataList;
	FolderShortcutsWin      m_shortcutsWin;
	Layout                  m_lo;

	clPtr<FS>*              m_fs;
	FSPath*                 m_path;

public:
	FolderShortcutsDlg( NCDialogParent* parent, clPtr<FS>* fp, FSPath* pPath )
		: PathListDlg( parent, m_shortcutsWin, _LT( "Folder Shortcuts" ), buttonsData )
		, m_shortcutsWin( this, m_dataList )
		, m_lo( 10, 10 )
		, m_fs( fp )
		, m_path( pPath )
	{
		m_ListWin.Show();
		m_ListWin.Enable();

		m_lo.AddWin( &m_shortcutsWin, 0, 0, 9, 0 );
		m_lo.SetLineGrowth( 9 );
		AddLayout( &m_lo );

		SetPosition();
		m_shortcutsWin.SetFocus();
	}

	virtual ~FolderShortcutsDlg() {}

	virtual bool Command( int id, int subId, Win* win, void* data );

	virtual bool OnKey( cevent_key* pEvent )
	{
		if ( pEvent->Type() == EV_KEYDOWN )
		{
			if ( pEvent->Key() == VK_INSERT )
			{
				Command( CMD_PLUS, 0, this, 0 );
				return true;
			}

			if ( pEvent->Key() == VK_DELETE )
			{
				Command( CMD_MINUS, 0, this, 0 );
				return true;
			}
		}

		return PathListDlg::OnKey( pEvent );
	}
};

bool FolderShortcutsDlg::Command( int id, int subId, Win* win, void* data )
{
	if ( id == CMD_MINUS )
	{
		const PathList::Data* data = m_ListWin.GetCurrentData();

		if ( !data || !data->name.data() )
		{
			return true;
		}

		if ( NCMessageBox( ( NCDialogParent* )Parent(), _LT( "Delete item" ),
		                   carray_cat<char>( _LT( "Delete '" ), unicode_to_utf8( data->name.data() ).data(), "' ?" ).data(),
		                   false, bListOkCancel ) == CMD_OK )
		{
			m_ListWin.DeleteCurrentItem();
		}

		return true;
	}

	if ( id == CMD_RENAME )
	{
		const PathList::Data* data = m_ListWin.GetCurrentData();

		if ( !data || !data->name.data() )
		{
			return true;
		}

		std::vector<unicode_t> name = InputStringDialog( ( NCDialogParent* )Parent(),
		                                                 utf8_to_unicode( _LT( "Rename item" ) ).data(), data->name.data() );

		if ( name.data() )
		{
			m_ListWin.RenameCurrentItem( name.data() );
		}

		return true;
	}

	if ( id == CMD_PLUS )
	{
		if ( m_fs && m_fs->Ptr() && m_path )
		{
			PathList::Data data;

			if ( !PathListFSToData( data, m_fs, m_path ) )
			{
				return false;
			}

			std::vector<unicode_t> name = InputStringDialog( ( NCDialogParent* )Parent(),
			                                                 utf8_to_unicode( _LT( "Enter shortcut name" ) ).data(), data.name.data() );

			if ( name.data() )
			{
				m_ListWin.InsertItem( name.data(), data.conf );
			}
		}

		return true;
	}

	return PathListDlg::Command( id, subId, win, data );
}


bool FolderShortcutDlg( NCDialogParent* parent, clPtr<FS>* fp, FSPath* pPath )
{
	FolderShortcutsDlg dlg( parent, fp, pPath );
	dlg.SetEnterCmd( 0 );

	const int res = dlg.DoModal();
	const PathList::Data* data = dlg.GetSelected();

	if ( res == CMD_OK && data && fp && pPath )
	{
		return PathListDataToFS( data, fp, pPath );
	}

	return false;
}
