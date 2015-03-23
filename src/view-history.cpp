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


#define IS_EDIT			"IS_EDIT"
#define VIEW_POS			"VIEW_POS"
#define EDIT_FIRST_LINE	"EDIT_FIRST_LINE"
#define EDIT_POINT_LINE	"EDIT_POINT_LINE"
#define EDIT_POINT_POS	"EDIT_POINT_POS"

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

void AddFileToHistory( const int CurrIndex, const PathList::Data& Data )
{
	if ( CurrIndex != -1 )
	{
		// remove existing item
		g_ViewList.Remove( CurrIndex );
	}

	// add item to the end of the list
	g_ViewList.Add( Data.name.data(), Data.conf );

	// limit number of elements in the list
	while ( g_ViewList.GetCount() > MAX_VIEW_HISTORY_COUNT )
	{
		g_ViewList.Remove( 0 );
	}
}

void AddFileViewToHistory( const int CurrIndex, PathList::Data& Data, const int Pos )
{
	// add file view properties
	StrConfig* Cfg = Data.conf.ptr();
	Cfg->Set( VIEW_POS, Pos );
	Cfg->Set( IS_EDIT, 0 );

	// add new item to the end of list
	AddFileToHistory( CurrIndex, Data );
}

int GetCreateFileViewPosHistory( clPtr<FS>* Fs, FSPath* Path )
{
	if ( !g_WcmConfig.editSavePos || !Fs || !Fs->Ptr() || !Path )
	{
		return -1;
	}
	
	std::vector<unicode_t> Name = new_unicode_str( (*Fs)->Uri( *Path ).GetUnicode() );
	
	// check if item already exists in the list
	const int Index = g_ViewList.FindByName( Name.data() );
	if ( Index != -1 )
	{
		StrConfig* Cfg = g_ViewList.GetData( Index )->conf.ptr();
		return Cfg->GetIntVal( VIEW_POS );
	}
	
	// collect all needed path information
	PathList::Data Data;
	if ( PathListFSToData( Data, Fs, Path ) )
	{
		AddFileViewToHistory( -1, Data, 0 );
	}
	
	return 0;
}

void UpdateFileViewPosHistory( std::vector<unicode_t> Name, const int Pos )
{
	if ( !g_WcmConfig.editSavePos )
	{
		return;
	}
	
	// check if item already exists in the list
	const int Index = g_ViewList.FindByName( Name.data() );
	if ( Index != -1 )
	{
		// copy current data
		PathList::Data Data = *g_ViewList.GetData( Index );

		// update data in the history list
		AddFileViewToHistory( Index, Data, Pos );
	}
}

void AddFileEditToHistory( const int CurrIndex, PathList::Data& Data, const int FirstLine, const int Line, const int Pos )
{
	// add file edit properties
	StrConfig* Cfg = Data.conf.ptr();
	Cfg->Set( EDIT_FIRST_LINE, FirstLine );
	Cfg->Set( EDIT_POINT_LINE, Line );
	Cfg->Set( EDIT_POINT_POS, Pos );
	Cfg->Set( IS_EDIT, 1 );

	// add new item to the end of list
	AddFileToHistory( CurrIndex, Data );
}

bool GetCreateFileEditPosHistory( clPtr<FS>* Fs, FSPath* Path, sEditorScrollCtx& Ctx )
{
	if ( !g_WcmConfig.editSavePos || !Fs || !Fs->Ptr() || !Path )
	{
		return false;
	}
	
	std::vector<unicode_t> Name = new_unicode_str( (*Fs)->Uri( *Path ).GetUnicode() );
	
	// check if item already exists in the list
	const int Index = g_ViewList.FindByName( Name.data() );
	if ( Index != -1 )
	{
		StrConfig* Cfg = g_ViewList.GetData( Index )->conf.ptr();
		Ctx.m_FirstLine = Cfg->GetIntVal( EDIT_FIRST_LINE );
		Ctx.m_Point.line = Cfg->GetIntVal( EDIT_POINT_LINE );
		Ctx.m_Point.pos = Cfg->GetIntVal( EDIT_POINT_POS );
		return true;
	}
	
	// collect all needed path information
	PathList::Data Data;
	if ( PathListFSToData( Data, Fs, Path ) )
	{
		AddFileEditToHistory( Index, Data, 0, 0, 0 );
	}
	
	return false;
}

void UpdateFileEditPosHistory( std::vector<unicode_t> Name, const sEditorScrollCtx& Ctx )
{
	if ( !g_WcmConfig.editSavePos )
	{
		return;
	}
	
	// check if item already exists in the list
	const int Index = g_ViewList.FindByName( Name.data() );
	if ( Index != -1 )
	{
		// copy current data
		PathList::Data Data = *g_ViewList.GetData( Index );

		// update data in the history list
		AddFileEditToHistory( Index, Data, Ctx.m_FirstLine, Ctx.m_Point.line, Ctx.m_Point.pos );
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
