/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * walcommander@linderdaum.com
 */

#pragma once

#if !defined( __STDC_FORMAT_MACROS )
#	define __STDC_FORMAT_MACROS
#endif

#if !defined(_MSC_VER) || _MSC_VER >= 1700
#  include <inttypes.h>
#endif

#include <vector>

#include "System/Platform.h"
#include "System/Types.h"

#include "wal.h"
#include "wal_sys_api.h"

/// return the position of the SubStr in a range
std::vector<unicode_t>::iterator FindSubstr( const std::vector<unicode_t>::iterator& begin, const std::vector<unicode_t>::iterator& end, const std::vector<unicode_t>& SubStr );

/// replace special symbols !.! in the command with the specified file name, return the resulting command
std::vector<unicode_t> MakeCommand( const std::vector<unicode_t>& Command, const unicode_t* FileName );

template<class T> inline const T* find_right_char( const T* s, T c )
{
	const T* p = 0;

	if ( s )
		for ( ; *s; s++ ) if ( *s == c ) { p = s; }

	return p;
}

/// get file extension
std::string GetFileExt( const unicode_t* uri );

/// convert UTF-32 to UTF-16
std::vector<wchar_t> UnicodeToUtf16( const unicode_t* s );

/// convert UTF-16 to UTF-32
std::vector<unicode_t> Utf16ToUnicode( const wchar_t* s );

/// convert UTF-8 to UCS-2
std::wstring widen( const std::string& utf8 );

/// convert UCS-2 to UTF-8
std::string narrow( const std::wstring& ucs2 );

std::vector<unicode_t> TruncateToLength( const std::vector<unicode_t>& Str, size_t MaxLength_Chars, bool InsertEllipsis );

inline std::vector<wchar_t> new_wchar_str( const wchar_t* str )
{
	if ( !str ) { return std::vector<wchar_t>(); }

	int l = 0;

	for ( const wchar_t* s = str; *s; s++ ) { l++; }

	std::vector<wchar_t> p( l + 1 );
	wchar_t* t;

	for ( t = p.data(); *str; t++, str++ ) { *t = *str; }

	*t = 0;
	return p;
}

std::string ToString( uint64_t FromUInt64 );
std::string ToString( int64_t FromInt64 );
std::string ToString( int FromInt );

// convert unsigned integer 12345678 to "12 345 678"
std::string ToStringGrouped( uint64_t FromUInt64, const char* GroupSeparator = " " );

std::string GetFormattedString( const char* Pattern, ... );

template <class T> inline  int carray_len( const T* s )
{
	for ( int i = 0; ; i++ )  if ( !*( s++ ) ) { return i; }
}

#define X(i) int n ## i = a ## i ? carray_len<T>(a ## i) : 0;
#define CP(i) if (n ## i) { for ( ;* a ## i; a ## i++) *(s++) = *(a ## i); }

template <class T> inline std::vector<T> carray_cat( const T* a1, const T* a2 )
{
	X( 1 );
	X( 2 );
	std::vector<T> str( n1 + n2 + 1 );
	T* s = str.data();
	CP( 1 );
	CP( 2 );
	*s = 0;
	return str;
}

template <class T> inline  std::vector<T> carray_cat( const T* a1, const T* a2, const T* a3 )
{
	X( 1 );
	X( 2 );
	X( 3 );
	std::vector<T> str( n1 + n2 + n3 + 1 );
	T* s = str.data();
	CP( 1 );
	CP( 2 );
	CP( 3 );
	*s = 0;
	return str;
}

template <class T> inline  std::vector<T> carray_cat( const T* a1, const T* a2, const T* a3, const T* a4 )
{
	X( 1 );
	X( 2 );
	X( 3 );
	X( 4 );
	std::vector<T> str( n1 + n2 + n3 + n4 + 1 );
	T* s = str.data();
	CP( 1 );
	CP( 2 );
	CP( 3 );
	CP( 4 );
	*s = 0;
	return str;

}

template <class T> inline  std::vector<T> carray_cat( const T* a1, const T* a2, const T* a3, const T* a4, const T* a5 )
{
	X( 1 );
	X( 2 );
	X( 3 );
	X( 4 );
	X( 5 );
	std::vector<T> str( n1 + n2 + n3 + n4 + n5 + 1 );
	T* s = str.data();
	CP( 1 );
	CP( 2 );
	CP( 3 );
	CP( 4 );
	CP( 5 );
	*s = 0;
	return str;
}

template <class T> inline  std::vector<T> carray_cat( const T* a1, const T* a2, const T* a3, const T* a4, const T* a5, const T* a6 )
{
	X( 1 );
	X( 2 );
	X( 3 );
	X( 4 );
	X( 5 );
	X( 6 );
	std::vector<T> str( n1 + n2 + n3 + n4 + n5 + n6 + 1 );
	T* s = str.data();
	CP( 1 );
	CP( 2 );
	CP( 3 );
	CP( 4 );
	CP( 5 );
	CP( 6 );
	*s = 0;
	return str;
}

#undef X
#undef CP
