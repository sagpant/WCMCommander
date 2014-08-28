/*
   Copyright (c) by Valery Goryachev (Wal)
*/


#include "vfspath.h"


/////////////////////////////////////////////// FSPath///////////////////////////////


void FSPath::SetItem( int n, int cs, const void* v ) // n>=0 && n<=Count(); если n == Count то в конец добавляется еще один элемент
{
	if ( n < 0 || n > Count() ) { return; }

	cacheCs = -2;

	if ( n == Count() ) { data.append( FSString( cs, v ) ); }
	else { data[n] = FSString( cs, v ); }
}

void FSPath::SetItemStr( int n, const FSString& str ) // n>=0 && n<=Count(); если n == Count то в конец добавляется еще один элемент
{
	if ( n < 0 || n > Count() ) { return; }

	cacheCs = -2;

	//чтоб не сработал эффект cptr
	FSString s;
	s.Copy( str );

	if ( n == Count() ) { data.append( s ); }
	else { data[n] = s; }
}


void FSPath::Copy( const FSPath& a )
{
	cacheCs = -2;
	data.clear();

	for ( int i = 0 ; i < a.Count(); i++ )
	{
		FSString s;
		s.Copy( a.data.const_item( i ) );
		data.append( s );
	}
}


void FSPath::_Set( int cs, const void* v )
{
	data.clear();
	cacheCs = -2;

	if ( cs == CS_UNICODE )
	{
		unicode_t* p = ( unicode_t* )v;

		if ( !*p ) { return; }

		while ( *p )
		{
			unicode_t* t = p;

			while ( *t != '\\' && *t != '/' && *t ) { t++; }

			int len = t - p;
			data.append( FSString( cs, p, len ) );
			p = ( *t ) ? t + 1 : t;
		}
	}
	else
	{
		char* p = ( char* )v;

		if ( !p || !*p ) { return; }

		while ( *p )
		{
			char* t = p;

			while ( *t != '\\' && *t != '/' && *t ) { t++; }

			int len = t - p;

			data.append( FSString( cs, p, len ) );
			p = ( *t ) ? t + 1 : t;
		}
	}
}

void FSPath::MakeCache( int cs, int splitter )
{
	ASSERT( data.count() >= 0 );

	if ( Count() == 1 && data[0].IsEmpty() ) // значит просто "/"
	{
		if ( cs == CS_UNICODE )
		{
			SetCacheSize( 2 * sizeof( unicode_t ) );
			( ( unicode_t* )cache.data() )[0] = splitter;
			( ( unicode_t* )cache.data() )[1] = 0;
		}
		else
		{
			SetCacheSize( 2 );
			cache[0] = splitter;
			cache[1] = 0;
		}

		cacheCs = cs;
		cacheSplitter = splitter;
		return;
	}

	int i, l;

	if ( cs == CS_UNICODE )
	{
		for ( i = l = 0; i < data.count(); i++ )
		{
			l += unicode_strlen( ( unicode_t* )data[i].Get( cs ) );
		}

		l += data.count() - 1; //разделители

		if ( l < 0 ) { l = 0; }

		SetCacheSize( ( l + 1 )*sizeof( unicode_t ) );

		unicode_t* p = ( unicode_t* ) cache.data();

		for ( i = 0; i < data.count(); i++ )
		{
			const void* v = data[i].Get( cs );
			l = unicode_strlen( ( unicode_t* )v );

			if ( l ) { memcpy( p, v, l * sizeof( unicode_t ) ); }

			p += l;

			if ( i + 1 < data.count() ) { *( p++ ) = splitter; }      /////////////// !!!
		}

		*p++ = 0;
	}
	else
	{
		for ( i = l = 0; i < data.count(); i++ )
		{
			l += strlen( ( char* )data[i].Get( cs ) );
		}

		l += data.count() - 1; //разделители

		if ( l < 0 ) { l = 0; }

		SetCacheSize( ( l + 1 )*sizeof( char ) );

		char* p = cache.data();

		for ( i = 0; i < data.count(); i++ )
		{
			const void* v = data[i].Get( cs );
			l = strlen( ( char* )v );

			if ( l ) { strcpy( p, ( char* )v ); }

			p += l;

			if ( i + 1 < data.count() ) { *( p++ ) = splitter; }        /////////////// !!!
		}

		*p++ = 0;
	}

	cacheCs = cs;
	cacheSplitter = splitter;
}

FSPath::~FSPath() {}


////////////////////////////////// cs_string /////////////////////////////////////////
inline cs_string::Node* new_node( int size, int cs )
{
	cs_string::Node* p = ( cs_string::Node* ) new char[sizeof( cs_string::Node ) + size];
	p->size = size;
	p->cs = cs;
	return p;
}

inline char* new_node( int cs, const char* s )
{
	if ( !s || s[0] == 0 ) { return 0; }

	int l = strlen( s );
	int size = l + 1;
	cs_string::Node* p = new_node( size, cs );
	memcpy( p->data, s, size );
	return ( char* ) p;
}

inline char* new_node( int cs, const char* s, int len )
{
	if ( len <= 0 ) { return 0; }

	int size = len + 1;
	cs_string::Node* p  = new_node( size, cs );
	memcpy( p->data, s, len );
	p->data[len] = 0;
	return ( char* ) p;
}

char* new_node( const unicode_t* s )
{
	if ( !s || s[0] == 0 ) { return 0; }

	const unicode_t* t;

	for ( t = s; *t; ) { t++; }

	int len = t - s;

	int size = ( len + 1 ) * sizeof( unicode_t );
	cs_string::Node* p = new_node( size, CS_UNICODE );
	memcpy( p->data, s, size );
	return ( char* ) p;
}

inline char* new_node( const unicode_t* s, int len )
{
	if ( len <= 0 ) { return 0; }

	int size = ( len + 1 ) * sizeof( unicode_t );
	cs_string::Node* p = new_node( size, CS_UNICODE );
	memcpy( p->data, s, len * sizeof( unicode_t ) );
	( ( unicode_t* )p->data )[len] = 0;
	return ( char* ) p;
}

cs_string::cs_string( const unicode_t* s ): data( new_node( s ) ) {}
cs_string::cs_string( const char* utf8Str ): data( new_node( CS_UTF8, utf8Str ) ) {}

void cs_string::copy( const cs_string& a )
{
	clear();

	if ( a.data )
	{
		int dataSize = sizeof( Node ) + ( ( Node* )a.data )->size;
		data = new char[dataSize];
		memcpy( data, a.data, dataSize );
	}
}

void cs_string::set( const unicode_t* s )
{
	char* p = new_node( s );

	if ( data ) { free_data(); }

	data = p;
}

void cs_string::set( int cs, const void* s )
{
	char* p = ( cs == CS_UNICODE ) ? new_node( ( const unicode_t* )s ) : new_node( cs, ( const char* )s ) ;

	if ( data ) { free_data(); }

	data = p;
}

void cs_string::set( int cs, const void* s, int len )
{
	char* p = ( cs == CS_UNICODE ) ? new_node( ( const unicode_t* )s, len ) : new_node( cs, ( const char* )s, len ) ;

	if ( data ) { free_data(); }

	data = p;
}


void cs_string::copy( const cs_string& a, int cs_id )
{
	if ( !a.data ) { clear(); return; }

	int acs = a.cs();

	if ( acs < CS_UNICODE || cs_id < CS_UNICODE )
	{
		fprintf( stderr, "BUG 1 cs_string::copy acs=%i, cs_id=%i\n", acs, cs_id );
		return;
	}


	if ( cs_id == acs )
	{
		copy( a );
	}
	else
	{
		unicode_t buf[0x100], *ptr = 0;
		unicode_t* u;
		ASSERT( acs >= -1 /*&& acs < CS_end*/ );
		ASSERT( cs_id >= -1 /*&& cs_id < CS_end*/ );

		try
		{
			clear();

			if ( acs == CS_UNICODE )
			{
				u = ( unicode_t* )( ( Node* )a.data )->data;
			}
			else
			{
				charset_struct* old_charset = charset_table[acs];
				int sym_count = old_charset->symbol_count( ( char* )( ( Node* )a.data )->data, -1 );

				if ( sym_count >= 0x100 )
				{
					ptr = new unicode_t[sym_count + 1];
					u = ptr;
				}
				else
				{
					u = buf;
				}

				unicode_t* CHK = old_charset->cs_to_unicode( u, ( char* )( ( Node* )a.data )->data, -1, 0 );
				u[sym_count] = 0;
			}

			if ( cs_id == CS_UNICODE )
			{
				data = new_node( u );
			}
			else
			{
				charset_struct* new_charset = charset_table[cs_id];

				int len = new_charset->string_buffer_len( u, -1 );
				data = ( char* )new_node( len + 1, cs_id ); //!!! параметры были переставлены :( фатально
				new_charset->unicode_to_cs( ( char* )( ( ( Node* )data )->data ), u, -1, 0 );
//ASSERT(len == strlen((char*)(((Node*)data)->data)));
			}

			if ( ptr )
			{
				delete [] ptr;
				ptr = 0;
			}

		}
		catch ( ... )
		{
			if ( ptr ) { delete [] ptr; }

			throw;
		}
	}
}


//////////////////////////////////////// FSString ///////////////////////////////////

extern unsigned  UnicodeLC( unsigned ch );

unicode_t FSString::unicode0 = 0;
char FSString::char0 = 0;


int FSString::Cmp( FSString& a )
{
	if ( !_primary.str() ) { return ( !a._primary.str() ) ? 0 : -1; }

	if ( !a._primary.str() ) { return 1; }

	return CmpStr<const unicode_t>( GetUnicode(), a.GetUnicode() );
}

int FSString::CmpNoCase( FSString& par )
{
	if ( !_primary.str() ) { return ( !par._primary.str() ) ? 0 : -1; }

	if ( !par._primary.str() ) { return 1; }

	const unicode_t* a = GetUnicode();
	const unicode_t* b = par.GetUnicode();
	unicode_t au = 0;
	unicode_t bu = 0;

	for ( ; *a; a++, b++ )
	{
		au = UnicodeLC( *a );
		bu = UnicodeLC( *b );

		if ( au != bu ) { break; }
	};

	return ( *a ? ( *b ? ( au < bu ? -1 : ( au == bu ? 0 : 1 ) ) : 1 ) : ( *b ? -1 : 0 ) );
}

void FSString::SetSys( const sys_char_t* p )
{
	_temp.clear();
#ifdef _WIN32
	int l = wcslen( p );
	carray<unicode_t> a( l + 1 );

	for ( int i = 0; i < l; i++ ) { a[i] = p[i]; }

	a[l] = 0;
	_primary.set( a.data() );
#else
	_primary.set( sys_charset_id, p );
#endif
}




