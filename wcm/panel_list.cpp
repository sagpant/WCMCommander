/*
   Copyright (c) by Valery Goryachev (Wal)
*/


#include "panel_list.h"
#include "wcm-config.h"

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
		list.alloc( n );

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
	if ( !list.ptr() || !listCount )
	{
		return;
	}

	FSNode** p = list.ptr();
	int count = listCount;

	switch ( sortMode )
	{
		case SORT_NAME:
			FSList::SortByName( p, count, ascSort, caseSensitive );
			break;

		case SORT_EXT:
			FSList::SortByExt ( p, count, ascSort, caseSensitive );
			break;

		case SORT_SIZE:
			FSList::SortBySize( p, count, ascSort );
			break;

		case SORT_MTIME:
			FSList::SortByMTime( p, count, ascSort );
			break;
	}
}


static bool accmask( const unicode_t* name, const unicode_t* mask )
{
	if ( !*mask )
	{
		return *name == 0;
	}

	while ( true )
	{
		switch ( *mask )
		{
			case 0:
				for ( ; *name ; name++ )
					if ( *name != '*' )
					{
						return false;
					}

				return true;

			case '?':
				break;

			case '*':
				while ( *mask == '*' ) { mask++; }

				if ( !*mask ) { return true; }

				for ( ; *name; name++ )
					if ( accmask( name, mask ) )
					{
						return true;
					}

				return false;

			default:
				if ( *name != *mask )
				{
					return false;
				}
		}

		name++;
		mask++;
	}
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

void PanelList::ShiftSelection( int n, int* selectType )
{
	if ( n <= 0 || n > listCount ) { return; }

	FSNode* p = list[n - 1];

	if ( !p ) { return; }

	if ( *selectType < 0 ) //not defined
	{
		*selectType = p->IsSelected() ? 0 : 1;
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
		FSNode* p = list.ptr()[i];

		if ( p->IsSelected() )
		{
			p->ClearSelected();
		}
		else
		{
			if ( !wcmConfig.panelSelectFolders && p->IsDir() ) { continue; }

			p->SetSelected();
			selectedCn.AddOne( p->st.size );
		}
	}
};
