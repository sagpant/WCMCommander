/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * wcm@linderdaum.com
 */

#include <stdint.h>
#include <cstdarg>
#include <string>
#include <algorithm>

#include "string-util.h"

static const int BUFFER = 65535;

std::vector<unicode_t>::iterator FindSubstr( const std::vector<unicode_t>::iterator& begin, const std::vector<unicode_t>::iterator& end, const std::vector<unicode_t>& SubStr )
{
	if ( begin >= end ) { return end; }

	return std::search( begin, end, SubStr.begin(), SubStr.end() );
}

std::vector<unicode_t> MakeCommand( const std::vector<unicode_t>& cmd, const unicode_t* FileName )
{
	std::vector<unicode_t> Result( cmd );
	std::vector<unicode_t> Name = new_unicode_str( FileName );

//	bool HasSpaces = StrHaveSpace( Name.data() );

	if ( Name.size() && Name.back() == 0 ) { Name.pop_back(); }

//	if ( HasSpaces )
	{
		Name.insert( Name.begin(), '"' );
		Name.push_back( '"' );
	}

	std::vector<unicode_t> Mask;
	Mask.push_back( '!' );
	Mask.push_back( '.' );
	Mask.push_back( '!' );

	auto pos = FindSubstr( Result.begin(), Result.end(), Mask );

	while ( pos != Result.end() )
	{
		pos = Result.erase( pos, pos + Mask.size() );
		size_t idx = pos - Result.begin();
		Result.insert( pos, Name.begin(), Name.end() );
		pos = Result.begin() + idx;
		pos = FindSubstr( pos + Name.size(), Result.end(), Mask );
	}

	return Result;
}

std::string GetFileExt( const std::string& uri )
{
	// FIXME: Probably non-utf8-proof.
	size_t dot = uri.rfind( '.' );
	if ( dot == std::string::npos )
	{
		return std::string();
	}
	return uri.substr(dot);
}

std::string GetFileExt( const unicode_t* uri )
{
	if ( !uri ) { return std::string(); }

	const unicode_t* ext = find_right_char<unicode_t>( uri, '.' );

	if ( !ext || !*ext ) { return std::string(); }

	return unicode_to_utf8_string( ext );
}

std::vector<wchar_t> UnicodeToUtf16( const unicode_t* s )
{
	if ( !s ) { return std::vector<wchar_t>(); }

	std::vector<wchar_t> p( unicode_strlen( s ) + 1 );
	wchar_t* d;

	for ( d = p.data(); *s; s++, d++ ) { *d = *s; }

	*d = 0;
	return p;
}

std::vector<unicode_t> Utf16ToUnicode( const wchar_t* s )
{
	std::vector<unicode_t> p( wcslen( s ) + 1 );
	unicode_t* d;

	for ( d = p.data(); *s; s++, d++ ) { *d = *s; }

	*d = 0;
	return p;
}

std::wstring widen( const std::string& utf8 )
{
#if defined(_WIN32)
	int Len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), utf8.length(), nullptr, 0);

	if ( Len > 0 )
	{
		wchar_t* Out = (wchar_t*)alloca( Len * sizeof(wchar_t) );
		
		MultiByteToWideChar( CP_UTF8, 0, utf8.c_str(), utf8.length(), Out, Len );

		return std::wstring( Out, Len );
	}

	return std::wstring();
#else
	return std::wstring( utf8str_to_unicode( utf8 ).data() );
#endif
}

// convert UCS-2 to UTF-8
std::string narrow( const std::wstring& ucs2 )
{
#if defined(_WIN32)
	std::string ret;

	int Len = WideCharToMultiByte( CP_UTF8, 0, ucs2.c_str(), ucs2.length(), nullptr, 0, nullptr, nullptr );

	if ( Len > 0 )
	{
		char* Out = (char*)alloca( Len );

		WideCharToMultiByte( CP_UTF8, 0, ucs2.c_str(), ucs2.length(), Out, Len, nullptr, nullptr );

		return std::string( Out, Len );
	}

	return std::string();
#else
	return unicode_to_utf8_string( ucs2.c_str() );
#endif
}

std::vector<unicode_t> TruncateToLength( const std::vector<unicode_t>& Str, size_t MaxLength_Chars, bool InsertEllipsis )
{
	size_t Length_Chars  = Str.size();

	if ( Length_Chars > MaxLength_Chars )
	{
		std::vector<unicode_t> Result = std::vector<unicode_t>( Str.begin() + ( Length_Chars - MaxLength_Chars ), Str.end() );

		// add ... at the beginning
		if ( InsertEllipsis )
		{
			const unicode_t Prefix[] = { '.', '.', '.' };
			Result.insert( Result.begin(), Prefix, Prefix + 3 );
		}

		return Result;
	}

	return Str;
}

inline std::string ToString( uint64_t FromUInt64 )
{
	char Buffer[ BUFFER ];

#ifdef OS_WINDOWS
#	if _MSC_VER >= 1400
	_ui64toa_s( FromUInt64, Buffer, sizeof(Buffer)-1, 10 );
#	else
	_ui64toa( FromUInt64, Buffer, 10 );
#	endif
#else
	Lsnprintf( Buffer, sizeof(Buffer)-1, "%" PRIu64, FromUInt64 );
#endif

	return std::string( Buffer );
}

std::string ToString( int64_t FromInt64 )
{
	char Buffer[ BUFFER ];

#ifdef OS_WINDOWS
#	if _MSC_VER >= 1400
	_i64toa_s( FromInt64, Buffer, sizeof(Buffer)-1, 10 );
#	else
	_i64toa( FromInt64, Buffer, 10 );
#	endif
#else
	Lsnprintf( Buffer, sizeof(Buffer)-1, "%" PRIi64, FromInt64 );
#endif

	return std::string( Buffer );
}

std::string ToString( int FromInt )
{
	char Buffer[ BUFFER ];

	Lsnprintf( Buffer, sizeof(Buffer) - 1, "%i", FromInt );

	return std::string( Buffer );
}

// convert unsigned integer 12345678 to "12 345 678"
std::string ToStringGrouped( uint64_t FromUInt64, const char* GroupSeparator )
{
	std::string Result = ToString( FromUInt64 );

	for ( int Pos = static_cast<int>( Result.length() ) - 3; Pos > 0; Pos -= 3 )
	{
		Result.insert( Pos, GroupSeparator );
	}

	return Result;
}

std::string GetFormattedString( const char* Pattern, ... )
{
	char Buffer[ BUFFER ];

	va_list p;
	va_start( p, Pattern );

	Lvsnprintf( Buffer, sizeof(Buffer) - 1, Pattern, p );

	va_end( p );

	return std::string( Buffer );
}
