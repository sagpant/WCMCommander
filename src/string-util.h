/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#pragma once

#include <vector>
#include "wal_sys_api.h"

template<class T> inline const T* find_right_char( const T* s, T c )
{
	const T* p = 0;

	if ( s )
		for ( ; *s; s++ ) if ( *s == c ) { p = s; }

	return p;
}

/// get file extension
inline std::string GetFileExt( const unicode_t* uri )
{
	if ( !uri ) { return std::string(); }

	const unicode_t* ext = find_right_char<unicode_t>( uri, '.' );

	if ( !ext || !*ext ) { return std::string(); }

	return unicode_to_utf8_string( ext );
}

inline std::vector<wchar_t> UnicodeToUtf16( const unicode_t* s )
{
	if ( !s ) { return std::vector<wchar_t>(); }

	std::vector<wchar_t> p( unicode_strlen( s ) + 1 );
	wchar_t* d;

	for ( d = p.data(); *s; s++, d++ ) { *d = *s; }

	*d = 0;
	return p;
}

inline std::vector<unicode_t> Utf16ToUnicode( const wchar_t* s )
{
	std::vector<unicode_t> p( wcslen( s ) + 1 );
	unicode_t* d;

	for ( d = p.data(); *s; s++, d++ ) { *d = *s; }

	*d = 0;
	return p;
}

// convert UTF-8 to UCS-2
inline std::wstring widen( const std::string& utf8 )
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
inline std::string narrow( const std::wstring& ucs2 )
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

inline std::vector<unicode_t> TruncateToLength( const std::vector<unicode_t>& Str, size_t MaxLength_Chars, bool InsertEllipsis )
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

template <class T> inline  std::vector<T> carray_cat( const T* a1, const T* a2, const T* a3, const T* a4, const T* a5, const T* a6, const T* a7 )
{
	X( 1 );
	X( 2 );
	X( 3 );
	X( 4 );
	X( 5 );
	X( 6 );
	X( 7 )
	std::vector<T> str( n1 + n2 + n3 + n4 + n5 + n6 + n7 + 1 );
	T* s = str.data();
	CP( 1 );
	CP( 2 );
	CP( 3 );
	CP( 4 );
	CP( 5 );
	CP( 6 );
	CP( 7 );
	*s = 0;
	return str;
}

template <class T> inline  std::vector<T> carray_cat(const T* a1, const T* a2, const T* a3, const T* a4, const T* a5, const T* a6, const T* a7, const T* a8)
{
	X(1);
	X(2);
	X(3);
	X(4);
	X(5);
	X(6);
	X(7)
	X(8)
		std::vector<T> str(n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8 + 1);
	T* s = str.data();
	CP(1);
	CP(2);
	CP(3);
	CP(4);
	CP(5);
	CP(6);
	CP(7);
	CP(8);
	*s = 0;
	return str;
}

#undef X
#undef CP
