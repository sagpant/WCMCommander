/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#include <wal.h>

#ifdef _WIN32

#include "ext-app.h"
#include "string-util.h"
#include "w32util.h"

static std::vector<wchar_t> GetOpenApp( wchar_t* ext )
{
	if ( !ext ) { return std::vector<wchar_t>(); }

	RegKey key;

	if ( !key.Open( HKEY_CURRENT_USER, carray_cat<wchar_t>( L"software\\classes\\", ext ).data() ) )
	{
		key.Open( HKEY_CLASSES_ROOT, ext );
	}

	std::vector<wchar_t> p = key.GetString();

	if ( !key.Open( HKEY_CURRENT_USER, carray_cat<wchar_t>( L"software\\classes\\", p.data(), L"\\shell\\open\\command" ).data() ) )
	{
		key.Open( HKEY_CLASSES_ROOT, carray_cat<wchar_t>( p.data(), L"\\shell\\open\\command" ).data() );
	}

	return key.GetString();
}

static std::vector<unicode_t> CfgStringToCommand( const wchar_t* cfgCmd, const unicode_t* uri )
{
	std::vector<unicode_t> res;
	int insCount = 0;

	int prev = 0;

	for ( const wchar_t* s = cfgCmd; *s; prev = *s, s++ )
	{
		if ( *s == '%' && ( s[1] == '1' || s[1] == 'l' || s[1] == 'L' ) )
		{
			if ( prev != '"' && prev != '\'' ) { res.push_back( '"' ); }

			for ( const unicode_t* t = uri; *t; t++ ) { res.push_back( *t ); }

			if ( prev != '"' && prev != '\'' ) { res.push_back( '"' ); }

			s++;
			insCount++;
		}
		else { res.push_back( *s ); }
	}

	if ( !insCount )
	{
		res.push_back( ' ' );
		res.push_back( '"' );

		for ( const unicode_t* t = uri; *t; t++ ) { res.push_back( *t ); }

		res.push_back( '"' );
	}

	res.push_back( 0 );

	return res;
}

static std::vector<wchar_t> GetFileExt( const unicode_t* uri )
{
	if ( !uri ) { return std::vector<wchar_t>(); }

	const unicode_t* ext = find_right_char<unicode_t>( uri, '.' );

	if ( !ext || !*ext ) { return std::vector<wchar_t>(); }

	return UnicodeToUtf16( ext );
}

static std::vector<unicode_t> NormalizeStr( unicode_t* s )
{
	if ( !s ) { return std::vector<unicode_t>(); }

	int n = unicode_strlen( s );
	std::vector<unicode_t> p( n + 1 );
	unicode_t* t = p.data();

	for ( ; *s; s++ ) if ( *s != '&' ) { *( t++ ) = *s; }

	*t = 0;
	return p;
}

clPtr<AppList> GetAppList( const unicode_t* uri )
{
	std::vector<wchar_t> ext = GetFileExt( uri );

	if ( !ext.data() ) { return 0; }

	RegKey key;

	if ( !key.Open( HKEY_CURRENT_USER, carray_cat<wchar_t>( L"software\\classes\\", ext.data() ).data() ) )
	{
		key.Open( HKEY_CLASSES_ROOT, ext.data() );
	}

	std::vector<wchar_t> p = key.GetString();

	RegKey key2;

	if ( !key2.Open( HKEY_CURRENT_USER, carray_cat<wchar_t>( L"software\\classes\\", p.data(), L"\\shell" ).data() ) )
	{
		key2.Open( HKEY_CLASSES_ROOT, carray_cat<wchar_t>( p.data(), L"\\shell" ).data() );
	}


	if ( !key2.Ok() ) { return 0; }

	clPtr<AppList> ret = new AppList();

	std::vector<wchar_t> pref = key2.GetString();

	for ( int i = 0; i < 10; i++ )
	{
		std::vector<wchar_t> sub = key2.SubKey( i );

		if ( !sub.data() ) { break; }

		RegKey key25;
		key25.Open( key2.Key(), sub.data() );

		if ( !key25.Ok() ) { continue; }

		std::vector<wchar_t> name = key25.GetString();
//wprintf(L"%s, %s\n", sub.ptr(), name.ptr());
		RegKey key3;
		key3.Open( key25.Key(), L"command" );
		std::vector<wchar_t> command = key3.GetString();

		if ( command.data() )
		{
			AppList::Node node;
			node.name = NormalizeStr( Utf16ToUnicode( name.data() && name[0] ? name.data() : sub.data() ).data() );
			node.cmd = CfgStringToCommand( command.data(), uri );

			bool IsPref = pref.data() && !_wcsicmp( pref.data(), sub.data() );
			bool IsOpen = !pref.data() && !_wcsicmp( L"Open", sub.data() );

			if ( IsPref || ( IsOpen && ret->list.count() > 0 ) )
			{
				ret->list.insert( 0 );
				ret->list[0] = node;
			}
			else
			{
				ret->list.append( node );
			}
		}
	}

	key2.Open( key.Key(), L"OpenWithList" );

	if ( key2.Ok() )
	{
		clPtr<AppList> openWith = new AppList();

		for ( int i = 0; i < 10; i++ )
		{
			std::vector<wchar_t> sub = key2.SubKey( i );

			if ( !sub.data() ) { break; }

			RegKey keyApplication;
			keyApplication.Open( HKEY_CLASSES_ROOT,
			                     carray_cat<wchar_t>( L"Applications\\", sub.data(), L"\\shell\\open\\command" ).data() );
			std::vector<wchar_t> command = keyApplication.GetString();

			if ( command.data() )
			{
				AppList::Node node;
				node.name = NormalizeStr( Utf16ToUnicode( sub.data() ).data() );
				node.cmd = CfgStringToCommand( command.data(), uri );
				openWith->list.append( node );
			}
		}

		if ( openWith->Count() > 0 )
		{
			AppList::Node node;
			static unicode_t openWidthString[] = { 'O', 'p', 'e', 'n', ' ', 'w', 'i', 't', 'h', 0};
			node.name = new_unicode_str( openWidthString );
			node.sub = openWith;
			ret->list.append( node );
		}
	}

	return ret->Count() ? ret : 0;
}


std::vector<unicode_t> GetOpenCommand( const unicode_t* uri, bool* needTerminal, const unicode_t** pAppName )
{
	std::vector<wchar_t> wCmd = GetOpenApp( GetFileExt( uri ).data() );

	if ( !wCmd.data() ) { return std::vector<unicode_t>(); }

	return CfgStringToCommand( wCmd.data(), uri );
}


#endif
