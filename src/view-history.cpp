/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#include "view-history.h"
#include "path-list.h"
#include "string-util.h"
#include "wcm-config.h"
#include "ltext.h"
#include "unicode_lc.h"


#define MAX_VIEW_HISTORY_COUNT   50

static const char ViewHistorySection[] = "ViewHistory";

PathList g_ViewList;


void LoadViewHistory()
{
	std::vector<std::string> List;
	LoadStringList( ViewHistorySection, List );

	g_ViewList.SetStrings( List );
}

void SaveViewHistory()
{
	std::vector<std::string> List;
	g_ViewList.GetStrings( List );

	SaveStringList( ViewHistorySection, List );
}

void AddFileToHistory( clPtr<FS>* fs, FSPath* path )
{
	if ( !fs || !fs->Ptr() || !path )
	{
		return;
	}

	// collect all needed path information
	PathList::Data data;
	
	if ( PathListFSToData( data, fs, path ) )
	{
		// check if item already exists in the list
		const int index = g_ViewList.FindByName( data.name.data() );
		
		if ( index != -1 )
		{
			// remove existing item
			g_ViewList.Remove( index );
		}

		// add item to the and of the list
		g_ViewList.Add( data.name.data(), data.conf );

		// limit number of elements in the list
		while ( g_ViewList.GetCount() > MAX_VIEW_HISTORY_COUNT )
		{
			g_ViewList.Remove( 0 );
		}
	}
}


class clViewHistoryDlg : public PathListDlg
{
private:
	PathListWin     m_historyWin;
	Layout          m_lo;

public:
	clViewHistoryDlg( NCDialogParent* parent, PathList& dataList )
	 : PathListDlg( parent, m_historyWin, _LT( "File View History" ), 0 )
	 , m_historyWin( this, dataList )
	 , m_lo( 10, 10 )
	{
		m_ListWin.Show();
		m_ListWin.Enable();

		m_lo.AddWin( &m_ListWin, 0, 0, 9, 0 );
		m_lo.SetLineGrowth( 9 );
		AddLayout( &m_lo );

		SetPosition();
		m_ListWin.SetFocus();

		if ( dataList.GetCount() > 0 )
		{
			// select the last item and make it visible
			m_ListWin.MoveCurrent( dataList.GetCount() - 1 );
		}
	}
};


bool ViewHistoryDlg( NCDialogParent* parent, clPtr<FS>* fp, FSPath* pPath )
{
	clViewHistoryDlg dlg( parent, g_ViewList );
	dlg.SetEnterCmd( 0 );

	const int res = dlg.DoModal();
	const PathList::Data* data = dlg.GetSelected();

	if ( res == CMD_OK && data && fp && pPath )
	{
		//return PathListDataToFS( data, fp, pPath );
	}

	return false;
}
