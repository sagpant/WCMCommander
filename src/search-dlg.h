/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#pragma once

#include <vector>

struct SearchAndReplaceParams
{
	/// if not zero, used to start immediate search in the viewer
	unicode_t m_SearchChar;
	/// file mask, used in the Alt+F7 search dialog
	std::vector<unicode_t> m_SearchMask;
	/// string to search, shared across all search dialogs in the app
	std::vector<unicode_t> m_SearchText;
	std::vector<unicode_t> m_ReplaceTo;
	bool m_CaseSensitive;
	SearchAndReplaceParams()
	 : m_SearchChar( 0 )
	 , m_SearchMask( {'*',0} )
	 , m_SearchText()
	 , m_ReplaceTo()
	 , m_CaseSensitive( false )
	{
	}
	SearchAndReplaceParams( const SearchAndReplaceParams& a )
	 : m_SearchChar( a.m_SearchChar )
	 , m_SearchMask( a.m_SearchMask )
	 , m_SearchText( a.m_SearchText )
	 , m_ReplaceTo( a.m_ReplaceTo )
	 , m_CaseSensitive( a.m_CaseSensitive )
	{
	}
	SearchAndReplaceParams& operator = ( const SearchAndReplaceParams& a )
	{
		SearchAndReplaceParams Tmp( a );

		this->swap( Tmp );

		return *this;
	}
	void swap( SearchAndReplaceParams& a )
	{
		std::swap( m_SearchChar, a.m_SearchChar );
		std::swap( m_SearchMask, a.m_SearchMask );
		std::swap( m_SearchText, a.m_SearchText );
		std::swap( m_ReplaceTo, a.m_ReplaceTo );
		std::swap( m_CaseSensitive, a.m_CaseSensitive );
	}
};

bool DoSearchDialog( NCDialogParent* parent, SearchAndReplaceParams* params );
bool DoFileSearchDialog( NCDialogParent* parent, SearchAndReplaceParams* params );
bool DoReplaceEditDialog( NCDialogParent* parent, SearchAndReplaceParams* params );
