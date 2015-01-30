/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#include "panel_list.h"
#include "wcm-config.h"
#include "globals.h"
#include "strmasks.h"
using namespace wal;

unicode_t PanelList::emptyStr[] = {0};
unicode_t PanelList::upperStr[] = {'.', '.', 0};

void PanelList::MakeList()
{
	listCount = 0;
	list.clear();

	filesCnNoDirs.Clear();
	filesCn.Clear();
	selectedCn.Clear();
	hiddenCn.Clear();

	if ( data.ptr() && data->Count() > 0 )
	{
		int n = data->Count();
		list.resize( n );

		int i = 0;

		for ( FSNode* p = data->First(); p && i < n; p = p->next )
			if ( showHidden || !p->IsHidden() )
			{
				list[ i++ ] = p;
				filesCn.AddOne( p->st.size );

				if ( p->IsSelected() ) { selectedCn.AddOne( p->st.size ); }

				if ( !p->IsDir() ) { filesCnNoDirs.AddOne( p->st.size ); }
			}
			else
			{
				p->ClearSelected();
				hiddenCn.AddOne( p->st.size );
			}

		listCount = i;
	}
}

void PanelList::Sort()
{
	if ( !list.data() || !listCount )
	{
		return;
	}
	FSNodeVectorSorter::Sort(list, ascSort, caseSensitive, sortMode);
}

void PanelList::Mark( const unicode_t* mask, bool enable )
{
	int n = listCount;

	PanelCounter counter;

	for ( int i = 0; i < n; i++ )
	{
		FSNode* p = list[i];

		if ( !p ) { continue; }

		bool ok = accmask( p->GetUnicodeName(), mask );

		if ( ok )
		{
			if ( enable )
			{
				p->SetSelected();
			}
			else
			{
				p->ClearSelected();
			}
		}

		if ( p->IsSelected() ) { counter.AddOne( p->Size() ); }
	}

	selectedCn = counter;
}

void PanelList::ShiftSelection( int n, LPanelSelectionType* selectType, bool RootDir )
{
	FSNode* p = NULL;

	if ( RootDir )
	{
		if ( n < 0 || n >= listCount ) { return; }

		p = list[n];
	}
	else
	{
		if ( n <= 0 || n > listCount ) { return; }

		p = list[n - 1];
	}

	if ( !p ) { return; }

	if ( *selectType < 0 ) //not defined
	{
		*selectType = p->IsSelected( ) ? LPanelSelectionType_Disable : LPanelSelectionType_Enable;
	}

	bool sel = ( *selectType > 0 );

	if ( sel == p->IsSelected() ) { return; }

	if ( sel )
	{
		p->SetSelected();
		selectedCn.AddOne( p->Size() );
	}
	else
	{
		p->ClearSelected();
		selectedCn.SubOne( p->Size() );
	}
}

void PanelList::InvertSelection()
{
	int n = listCount;
	selectedCn.Clear();

	for ( int i = 0; i < n; i++ )
	{
		FSNode* p = list.data()[i];

		if ( p->IsSelected() )
		{
			p->ClearSelected();
		}
		else
		{
			if ( !g_WcmConfig.panelSelectFolders && p->IsDir() ) { continue; }

			p->SetSelected();
			selectedCn.AddOne( p->st.size );
		}
	}
};
