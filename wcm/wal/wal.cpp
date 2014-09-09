/*
   Copyright (c) by Valery Goryachev (Wal)
*/

#include "wal.h"

namespace wal
{

	static void def_thread_error_func( int err, const char* msg, const char* file, int* line )
	{
		if ( !msg ) { msg = ""; }

		fprintf( stderr, "THREAD ERROR: %s\n", msg );
	}

	void ( *thread_error_func )( int err, const char* msg, const char* file, int* line ) = def_thread_error_func;


	std::vector<unicode_t> new_unicode_str( const unicode_t* s )
	{
		if ( !s ) { return std::vector<unicode_t>(); }

		const unicode_t* p;

		for ( p = s; *p; ) { p++; }

		int len = p - s;
		std::vector<unicode_t> r( len + 1 );

		if ( len ) { memcpy( r.data(), s, len * sizeof( unicode_t ) ); }

		r[len] = 0;
		return r;
	}

	std::vector<sys_char_t> new_sys_str( const sys_char_t* s )
	{
		if ( !s ) { return std::vector<sys_char_t>(); }

		int len = sys_strlen( s );
		std::vector<sys_char_t> r( len + 1 );

		if ( len ) { memcpy( r.data(), s, len * sizeof( sys_char_t ) ); }

		r[len] = 0;
		return r;
	}

	std::vector<char> new_char_str( const char* s )
	{
		if ( !s ) { return std::vector<char>(); }

		int len = strlen( s );
		std::vector<char> r( len + 1 );

		if ( len )
		{
			memcpy( r.data(), s, len * sizeof( char ) );
		}

		r[len] = 0;
		return r;
	}


	std::vector<sys_char_t> utf8_to_sys( const char* s )
	{
		if ( !s ) { return std::vector<sys_char_t>(); }

		int symbolCount = utf8_symbol_count( s );
		std::vector<unicode_t> unicodeBuf( symbolCount + 1 );
		utf8_to_unicode( unicodeBuf.data(), s );

		int sys_len = sys_string_buffer_len( unicodeBuf.data(), symbolCount );
		std::vector<sys_char_t> sysBuf( sys_len + 1 );
		unicode_to_sys( sysBuf.data(), unicodeBuf.data(), symbolCount );
		return sysBuf;
	};


	std::vector<char> sys_to_utf8( const sys_char_t* s )
	{
		if ( !s ) { return std::vector<char>(); }

		int symbolCount = sys_symbol_count( s );
		std::vector<unicode_t> unicodeBuf( symbolCount + 1 );
		sys_to_unicode( unicodeBuf.data(), s );
		int utf8Len = utf8_string_buffer_len( unicodeBuf.data(), symbolCount );
		std::vector<char> utf8Buf( utf8Len + 1 );
		unicode_to_utf8( utf8Buf.data(), unicodeBuf.data(), symbolCount );
		return utf8Buf;
	}


	std::vector<unicode_t> utf8_to_unicode( const char* s )
	{
		if ( !s ) { return std::vector<unicode_t>(); }

		int symbolCount = utf8_symbol_count( s );
		std::vector<unicode_t> unicodeBuf( symbolCount + 1 );
		utf8_to_unicode( unicodeBuf.data(), s );
		return unicodeBuf;
	}

	std::vector<char> unicode_to_utf8( const unicode_t* u )
	{
		if ( !u ) { return std::vector<char>(); }

		int size = utf8_string_buffer_len( u );
		std::vector<char> s( size + 1 );
		unicode_to_utf8( s.data(), u );
		return s;
	}

	bool unicode_is_equal( const unicode_t* s, const unicode_t* ss )
	{
		if ( !s || !ss ) return false;

		while ( *ss != 0 ) if ( *s++ != *ss++ )
		{
			return false;
		}

		if ( *ss == 0 && *s == 0 ) return true;

		return false;
	}

	bool unicode_starts_with_and_not_equal( const unicode_t* Str, const unicode_t* SubStr )
	{
		if ( !Str || !SubStr ) return false;

		const unicode_t* S = Str;
		const unicode_t* SS = SubStr;

		while ( *SS != 0 ) if ( *S++ != *SS++ )
		{
			return false;
		}

		if ( *SS == 0 && *S == 0 ) return false;

		return true;
	}

	int unicode_strlen(const unicode_t* s)
	{
		if ( !s ) return 0;

		const unicode_t* p = s;

		while (*p) { p++; }

		return p - s;
	}

	unicode_t* unicode_strchr(const unicode_t* s, unicode_t c)
	{
		while (*s != c && *s) { s++; }

		return (unicode_t*)(*s ? s : 0);
	}

	unicode_t* unicode_strcpy(unicode_t* d, const unicode_t* s)
	{
		if ( !d || !s ) return NULL;

		unicode_t* ret = d;
		while ((*d++ = *s++) != 0)
			;
		return ret;
	}

	// copy unlit end of string, or when n chars copid, whichever comes first. 
	// d is always 0-ended
	unicode_t* unicode_strncpy0(unicode_t* d, const unicode_t* s, int n)
	{
		if ( !d || !s ) return NULL;

		unicode_t* ret = d;
		for (;;)
		{
			if (n-- == 0)
			{
				*d = 0;
				break;
			}
			if ((*d++ = *s++) == 0)
			{
				break;
			}
		}
		return ret;
	}

	void unicode_strcat(unicode_t* d, const unicode_t* s)
	{
		if ( !d || !s ) return;

		while (*d)
			d++;
		while ((*d++ = *s++) != 0)
			;
	}

	unicode_t* unicode_strdup(const unicode_t* s)
	{
		return unicode_strcpy(new unicode_t[unicode_strlen(s) + 1], s);
	}


	////////////  system File implementation

	void File::Throw( const sys_char_t* name )
	{
		if ( !name ) { name = _fileName.data(); }

		static const char noname[] = "<NULL>";
		throw_syserr( 0, "'%s'", name ? sys_to_utf8( name ).data() : noname );
	}

	void File::Throw() { Throw( 0 ); }

}; //namespace wal
