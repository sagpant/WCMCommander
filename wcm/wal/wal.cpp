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


////////////  system File implementation

	void File::Throw( const sys_char_t* name )
	{
		if ( !name ) { name = _fileName.data(); }

		static const char noname[] = "<NULL>";
		throw_syserr( 0, "'%s'", name ? sys_to_utf8( name ).data() : noname );
	}

	void File::Throw() { Throw( 0 ); }

}; //namespace wal
