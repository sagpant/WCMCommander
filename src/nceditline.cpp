/*
* Part of WCM Commander
* https://github.com/corporateshark/WCMCommander
* wcm@linderdaum.com
*/

#include "nceditline.h"
#include "string-util.h"
#include "unicode_lc.h"
#include "nchistory.h"
#include "globals.h"


static int uiClassNCEditLine = GetUiID( "NCEditLine" );

int clNCEditLine::UiGetClassId()
{
	return uiClassNCEditLine;
}

clNCEditLine::clNCEditLine( const char* FieldName, int Id, Win* Parent, const unicode_t* Txt, int Cols, int Rows, crect* Rect )
	: ComboBox( Id, Parent, Cols, Rows, 0, Rect )
	, m_FieldName( FieldName )
{
}

bool clNCEditLine::EventKey( cevent_key* pEvent )
{
	if ( pEvent->Type() == EV_KEYDOWN && pEvent->Key() == VK_RETURN && IsBoxOpened() )
	{
		// close the box and propagate Enter key to parent dialog
		CloseBox();
		return false;
	}

	return ComboBox::EventKey( pEvent );
}

bool clNCEditLine::Command( int Id, int SubId, Win* Win, void* Data )
{
	if ( Id == CMD_EDITLINE_INFO && SubId == SCMD_EDITLINE_CHANGED && IsEditLine( Win ) && g_WcmConfig.systemAutoComplete )
	{
		bool HasHistory = false;
		std::vector<unicode_t> Text = GetText();
		HistCollect_t* List = GetFieldHistCollect( m_FieldName );
		if ( List )
		{
			Clear();
			static unicode_t Str0 = 0;
			Append( &Str0 ); // empty line goes first
			
			for ( int i = 0, count = List->size(); i < count; i++ )
			{
				const unicode_t* Str = List->at( i ).data();
				
				if ( unicode_starts_with_and_not_equal( Str, Text.data(), true ) )
				{
					Append( Str );
					HasHistory = true;
				}
			}
		}

		if ( HasHistory )
		{
			if ( !IsBoxOpened() )
			{
				OpenBox();
			}
			else
			{
				RefreshBox();
				InitBox();
			}
		}
		else
		{
			CloseBox();
			Clear();
		}
		
		return true;
	}

	return ComboBox::Command( Id, SubId, Win, Data );
}

void clNCEditLine::InitBox()
{
	if ( Count() > 0 )
	{
		MoveCurrent( 0, false );
		
		// reset saved prefix
		m_Prefix = std::vector<unicode_t>();
	}
}

void clNCEditLine::OnItemChanged(int ItemIndex)
{
	std::vector<unicode_t> Text = GetText();
	
	ComboBox::OnItemChanged( ItemIndex );
	
	if ( ItemIndex >= 0 && m_Prefix.size() == 0 )
	{
		// save user entered text
		m_Prefix = Text;
	}
	else if ( ItemIndex == 0 && m_Prefix.size() > 0 )
	{
		// restore user entered text
		SetText( m_Prefix.data() );
		
		// reset saved prefix
		m_Prefix = std::vector<unicode_t>();
	}
}

bool clNCEditLine::OnOpenBox()
{
	if ( Count() == 0 )
	{
		HistCollect_t* List = GetFieldHistCollect( m_FieldName );
		if ( List )
		{
			static unicode_t Str0 = 0;
			Append( &Str0 ); // empty line goes first
			
			for ( int i = 0, count = List->size(); i < count; i++ )
			{
				Append( List->at( i ).data() );
			}
			
			RefreshBox();
		}
	}

	InitBox();
	return true;
}

void clNCEditLine::AddCurrentTextToHistory()
{
	std::vector<unicode_t> Text = GetText();
	
	AddFieldTextToHistory( m_FieldName, Text.data() );
}
