/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * walcommander@linderdaum.com
 */

#include "strmasks.h"

bool accmask_nocase( const unicode_t* name, const unicode_t* mask )
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
				return true;

			case '?':
				break;

			case '*':
				while ( *mask == '*' ) { mask++; }

				if ( !*mask ) { return true; }

				for ( ; *name; name++ )
					if ( accmask_nocase( name, mask ) )
					{
						return true;
					}

				return false;

			default:
				if ( UnicodeLC( *name ) != UnicodeLC( *mask ) )
				{
					return false;
				}
		}

		name++;
		mask++;
	}
}

bool accmask( const unicode_t* name, const unicode_t* mask )
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

clMultimaskSplitter::clMultimaskSplitter( const std::vector<unicode_t>& MultiMask )
	: m_MultiMask( MultiMask )
	, m_CurrentPos( 0 )
{}

bool clMultimaskSplitter::HasNextMask() const
{
	return m_CurrentPos < m_MultiMask.size();
}

std::vector<unicode_t> clMultimaskSplitter::GetNextMask()
{
	size_t Next = m_CurrentPos;

	// find the nearest ','
	while ( Next < m_MultiMask.size() && m_MultiMask[Next] != ',' ) { Next++; }

	if ( m_CurrentPos == Next ) { return std::vector<unicode_t>(); }

	std::vector<unicode_t> Result( m_MultiMask.begin() + m_CurrentPos, m_MultiMask.begin() + Next );

	if ( Next < m_MultiMask.size() && m_MultiMask[Next] == ',' )
	{
		// skip ','
		Next++;

		// and trailing spaces
		while ( Next < m_MultiMask.size() && m_MultiMask[Next] <= ' ' ) { Next++; }
	}

	m_CurrentPos = Next;

	Result.push_back( 0 );

	return Result;
}

bool clMultimaskSplitter::CheckAndFetchAllMasks( const unicode_t* FileName )
{
	m_CurrentPos = 0;

	while ( HasNextMask() )
	{
		if (
#if defined( _WIN32 ) || defined( __APPLE__ )
		   accmask_nocase
#else
		   accmask
#endif
		   ( FileName, GetNextMask().data() ) )
		{
			return true;
		}
	}

	return false;
}

bool clMultimaskSplitter::CheckAndFetchAllMasks_NoCase( const unicode_t* FileName )
{
	m_CurrentPos = 0;

	while ( HasNextMask() )
	{
		if ( accmask_nocase( FileName, GetNextMask().data() ) ) { return true; }
	}

	return false;
}

bool clMultimaskSplitter::CheckAndFetchAllMasks_Case( const unicode_t* FileName )
{
	m_CurrentPos = 0;

	while ( HasNextMask() )
	{
		if ( accmask( FileName, GetNextMask().data() ) ) { return true; }
	}

	return false;
}
