/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * wcm@linderdaum.com
 */

#pragma once

#include "swl/swl.h"

template <typename ContainerItemType>
class iContainerListWin: public VListWin
{
public:
	iContainerListWin( Win* parent, std::vector<ContainerItemType>* Items )
		: VListWin( Win::WT_CHILD, WH_TABFOCUS | WH_CLICKFOCUS, 0, parent, VListWin::SINGLE_SELECT, VListWin::BORDER_3D, 0 )
		, m_ItemList( Items )
	{
		wal::GC gc( this );
		gc.Set( GetFont() );
		cpoint ts = gc.GetTextExtents( ABCString );
		//int fontW = ( ts.x / ABCStringLen );
		int fontH = ts.y + 2;

		this->SetItemSize( ( fontH > 16 ? fontH : 16 ) + 1, 100 );

		SetCount( m_ItemList->size() );
		SetCurrent( 0 );

		LSize ls;
		ls.x.maximal = 10000;
		ls.x.ideal = 500;
		ls.x.minimal = 300;

		ls.y.maximal = 10000;
		ls.y.ideal = 300;
		ls.y.minimal = 200;

		SetLSize( ls );
	}
	virtual ~iContainerListWin() {};

	const ContainerItemType* GetCurrentData( ) const
	{
		int n = GetCurrent( );

		if ( n < 0 || n >= ( int )m_ItemList->size() ) { return nullptr; }

		return &( m_ItemList->at( n ) );
	}

	ContainerItemType* GetCurrentData( )
	{
		int n = GetCurrent( );

		if ( n < 0 || n >= ( int )m_ItemList->size() ) { return nullptr; }

		return &( m_ItemList->at( n ) );
	}

	void Ins( const ContainerItemType& p )
	{
		m_ItemList->push_back( p );
		SetCount( m_ItemList->size( ) );
		SetCurrent( ( int )m_ItemList->size() - 1 );
		CalcScroll();
		Invalidate();
	}

	void Del()
	{
		int n = GetCurrent();

		if ( n < 0 || n >= ( int )m_ItemList->size( ) ) { return; }

		m_ItemList->erase( m_ItemList->begin( ) + n );

		SetCount( m_ItemList->size( ) );
		CalcScroll();
		Invalidate();
	}

	void Rename( const ContainerItemType& p )
	{
		int n = GetCurrent();

		if ( n < 0 || n >= ( int )m_ItemList->size( ) ) { return; }

		m_ItemList->at( n ) = p;
		CalcScroll();
		Invalidate();
	}

	virtual void DrawItem( wal::GC& gc, int n, crect rect ) = 0;

protected:
	std::vector<ContainerItemType>* m_ItemList;
};
