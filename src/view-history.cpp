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

void AddFileToHistory( const PathList::Data& Data )
{
	// add item to the end of the list
	g_ViewList.Add( Data.name.data(), Data.conf );

	// limit number of elements in the list
	while ( g_ViewList.GetCount() > MAX_VIEW_HISTORY_COUNT )
	{
		g_ViewList.Remove( 0 );
	}
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
		return Cfg->GetIntVal( "VIEW_POS" );
	}
	
	// collect all needed path information
	PathList::Data Data;
	
	if ( PathListFSToData( Data, Fs, Path ) )
	{
		StrConfig* Cfg = Data.conf.ptr();
		Cfg->Set( "VIEW_POS", (int) 0 );
		Cfg->Set( "IS_EDIT", (int) 0 );
		
		AddFileToHistory( Data );
	}
	
	return 0;
}

void UpdateFileViewPosHistory( std::vector<unicode_t> Name, int Pos )
{
	if ( !g_WcmConfig.editSavePos )
	{
		return;
	}
	
	// check if item already exists in the list
	const int Index = g_ViewList.FindByName( Name.data() );
	if ( Index == -1 )
	{
		return;
	}
	
	// copy current data and set new pos
	PathList::Data Data = *g_ViewList.GetData( Index );
	StrConfig* Cfg = Data.conf.ptr();
	Cfg->Set( "VIEW_POS", Pos );
	Cfg->Set( "IS_EDIT", (int) 0 );
		
	// remove existing item
	g_ViewList.Remove( Index );
	
	// add new item to the end of list
	AddFileToHistory( Data );
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
		Ctx.m_FirstLine = Cfg->GetIntVal( "EDIT_FIRST_LINE" );
		Ctx.m_Point.line = Cfg->GetIntVal( "EDIT_POINT_LINE" );
		Ctx.m_Point.pos = Cfg->GetIntVal( "EDIT_POINT_POS" );
		return true;
	}
	
	// collect all needed path information
	PathList::Data Data;
	
	if ( PathListFSToData( Data, Fs, Path ) )
	{
		StrConfig* Cfg = Data.conf.ptr();
		Cfg->Set( "EDIT_FIRST_LINE", (int) 0 );
		Cfg->Set( "EDIT_POINT_LINE", (int) 0 );
		Cfg->Set( "EDIT_POINT_POS", (int) 0 );
		Cfg->Set( "IS_EDIT", (int) 1 );
		
		AddFileToHistory( Data );
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
	if ( Index == -1 )
	{
		return;
	}
	
	// copy current data and set new pos
	PathList::Data Data = *g_ViewList.GetData( Index );
	StrConfig* Cfg = Data.conf.ptr();
	Cfg->Set( "EDIT_FIRST_LINE", Ctx.m_FirstLine );
	Cfg->Set( "EDIT_POINT_LINE", Ctx.m_Point.line );
	Cfg->Set( "EDIT_POINT_POS", Ctx.m_Point.pos );
	Cfg->Set( "IS_EDIT", (int) 1 );
	
	// remove existing item
	g_ViewList.Remove( Index );
	
	// add new item to the end of list
	AddFileToHistory( Data );
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
