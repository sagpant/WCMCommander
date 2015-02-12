/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#include "folder-history.h"
#include "path-list.h"
#include "string-util.h"
#include "wcm-config.h"
#include "ltext.h"
#include "unicode_lc.h"


#define MAX_FOLDERS_HISTORY_COUNT   50

static const char foldersHistorySection[] = "FoldersHistory";

PathList g_dataList;


void LoadFoldersHistory()
{
	std::vector<std::string> list;
	LoadStringList( foldersHistorySection, list );

	g_dataList.SetStrings( list );
}

void SaveFoldersHistory()
{
	std::vector<std::string> list;
	g_dataList.GetStrings( list );

	SaveStringList( foldersHistorySection, list );
}

void AddFolderToHistory( clPtr<FS>* fs, FSPath* path )
{
	if ( fs && fs->Ptr() && path )
	{
		// collect all needed path information
		PathList::Data data;

		if ( PathListFSToData( data, fs, path ) )
		{
			// check if item already exists in the list
			const int index = g_dataList.FindByName( data.name.data() );

			if ( index != -1 )
			{
				// remove existing item
				g_dataList.Remove( index );
			}

			// add item to the and of the list
			g_dataList.Add( data.name.data(), data.conf );

			// limit number of elements in the list
			while ( g_dataList.GetCount() > MAX_FOLDERS_HISTORY_COUNT )
			{
				g_dataList.Remove( 0 );
			}
		}
	}
}


class FoldersHistoryDlg : public PathListDlg
{
private:
	PathListWin     m_historyWin;
	Layout          m_lo;

public:
	FoldersHistoryDlg( NCDialogParent* parent, PathList& dataList )
		: PathListDlg( parent, m_historyWin, _LT( "Folders History" ), 0 ),
		  m_historyWin( this, dataList ),
		  m_lo( 10, 10 )
	{
		m_listWin.Show();
		m_listWin.Enable();

		m_lo.AddWin( &m_listWin, 0, 0, 9, 0 );
		m_lo.SetLineGrowth( 9 );
		AddLayout( &m_lo );

		SetPosition();
		m_listWin.SetFocus();

		if ( dataList.GetCount() > 0 )
		{
			// select the last item and make it visible
			m_listWin.MoveCurrent( dataList.GetCount() - 1 );
		}
	}

	virtual ~FoldersHistoryDlg() {}
};


bool FolderHistoryDlg( NCDialogParent* parent, clPtr<FS>* fp, FSPath* pPath )
{
	FoldersHistoryDlg dlg( parent, g_dataList );
	dlg.SetEnterCmd( 0 );

	const int res = dlg.DoModal();
	PathList::Data* data = dlg.GetSelected();

	if ( res == CMD_OK && data && fp && pPath )
	{
		return PathListDataToFS( data, fp, pPath );
	}

	return false;
}
