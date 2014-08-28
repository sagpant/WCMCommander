/*
   Copyright (c) by Valery Goryachev (Wal)
*/

#ifdef _WIN32
#include <winsock2.h>
#endif

#include "vfs-uri.h"
#include "vfs-smb.h"
#include "vfs-ftp.h"
#include "vfs-sftp.h"

#include "string-util.h"

static unicode_t rootPathStr[] = {'/', 0};

//FSPtr systemFSPtr = new FSSys();

inline const unicode_t* FindFirstChar( const unicode_t* s, unicode_t c )
{
	for ( ; *s; s++ ) if ( *s == c ) { return s; }

	return s;
}

static void SetString( char* dest, int len, const char* src )
{
	for ( ; *src && len > 1; dest++, src++, len-- ) { *dest = *src; }

	if ( len > 0 ) { *dest = 0; }
}

#ifdef LIBSMBCLIENT_EXIST

FSPtr ParzeSmbURI( const unicode_t* uri, FSPath& path, FSPtr* checkFS, int count )
{
	path.Set( rootPathStr );

	if ( !uri[0] ) { return new FSSmb(); }

	FSSmbParam param;

	const unicode_t* userDelimiter = FindFirstChar( uri, '@' );

	if ( *userDelimiter )
	{
		FSString s( CS_UNICODE, uri, userDelimiter - uri );
		SetString( const_cast<char*>( param.user ), sizeof( param.user ), s.GetUtf8() );
		uri = userDelimiter + 1;
	}

	const unicode_t* hostEnd = FindFirstChar( uri, '/' );
	FSString host( CS_UNICODE, uri, hostEnd - uri );
	SetString( const_cast<char*>( param.server ), sizeof( param.server ), host.GetUtf8() );

	uri = hostEnd;

	FSString link = uri;

	if ( !ParzeLink( path, link ) ) { return FSPtr(); }

	return new FSSmb( &param );
}

#endif


FSPtr ParzeFtpURI( const unicode_t* uri, FSPath& path, FSPtr* checkFS, int count )
{
	path.Set( rootPathStr );

	if ( !uri[0] ) { return FSPtr(); }

	FSFtpParam param;

	const unicode_t* userDelimiter = FindFirstChar( uri, '@' );

	if ( *userDelimiter )
	{
		param.user = FSString( CS_UNICODE, uri, userDelimiter - uri ).GetUnicode();
		uri = userDelimiter + 1;
		param.anonymous = false;
	}
	else
	{
		param.anonymous = true;
	}


	const unicode_t* host_port_End = FindFirstChar( uri, '/' );
	const unicode_t* host_End = FindFirstChar( uri, ':' );

	FSString host( CS_UNICODE, uri, ( ( host_End < host_port_End ) ? host_End :  host_port_End )  - uri );

	int port  = 0;

	for ( const unicode_t* s = host_End + 1; s < host_port_End; s++ )
		if ( *s >= '0' && *s <= '9' ) { port = port * 10 + ( *s - '0' ); }
		else { break; }

	if ( port > 0 && port < 65536 ) { param.port = port; }

	param.server = host.GetUnicode();

	uri = host_port_End;

	FSString link = uri;

	if ( !ParzeLink( path, link ) ) { return FSPtr(); }

	for ( int i = 0; i < count; i++ )
		if ( checkFS[i].Ptr() && checkFS[i]->Type() == FS::FTP )
		{
			FSFtp* p = ( FSFtp* )checkFS[i].Ptr();
			FSFtpParam checkParam;
			p->GetParam( &checkParam );

			if (  !CmpStr<const unicode_t>( param.server.Data(), checkParam.server.Data() ) &&
			      ( param.anonymous == checkParam.anonymous && ( param.anonymous || !CmpStr<const unicode_t>( param.user.Data(), checkParam.user.Data() ) ) ) &&
			      param.port == checkParam.port )
			{
				return checkFS[i];
			}
		}

	return new FSFtp( &param );
}

#if defined(LIBSSH_EXIST) || defined(LIBSSH2_EXIST)

FSPtr ParzeSftpURI( const unicode_t* uri, FSPath& path, FSPtr* checkFS, int count )
{
	path.Set( rootPathStr );

	if ( !uri[0] ) { return FSPtr(); }

	FSSftpParam param;

	const unicode_t* userDelimiter = FindFirstChar( uri, '@' );

	if ( *userDelimiter )
	{
		param.user = FSString( CS_UNICODE, uri, userDelimiter - uri ).GetUnicode();
		uri = userDelimiter + 1;
	}

	const unicode_t* host_port_End = FindFirstChar( uri, '/' );
	const unicode_t* host_End = FindFirstChar( uri, ':' );

	FSString host( CS_UNICODE, uri, ( ( host_End < host_port_End ) ? host_End :  host_port_End )  - uri );

	int port  = 0;

	for ( const unicode_t* s = host_End + 1; s < host_port_End; s++ )
		if ( *s >= '0' && *s <= '9' ) { port = port * 10 + ( *s - '0' ); }
		else { break; }

	if ( port > 0 && port < 65536 ) { param.port = port; }

	param.server = host.GetUnicode();

	uri = host_port_End;

	FSString link = uri;

	if ( !ParzeLink( path, link ) ) { return FSPtr(); }

	for ( int i = 0; i < count; i++ )
		if ( checkFS[i].Ptr() && checkFS[i]->Type() == FS::SFTP )
		{
			FSSftp* p = ( FSSftp* )checkFS[i].Ptr();
			FSSftpParam checkParam;

			p->GetParam( &checkParam );


			if (  !CmpStr<const unicode_t>( param.server.Data(), checkParam.server.Data() ) &&
			      !CmpStr<const unicode_t>( param.user.Data(), checkParam.user.Data() ) &&
			      param.port == checkParam.port )
			{
				return checkFS[i];
			}
		}

	return new FSSftp( &param );
}

#endif

FSPtr ParzeURI( const unicode_t* uri, FSPath& path, FSPtr* checkFS, int count )
{

#ifdef LIBSMBCLIENT_EXIST

	if ( uri[0] == 's' && uri[1] == 'm' && uri[2] == 'b' && uri[3] == ':' && uri[4] == '/' && uri[5] == '/' )
	{
		return ParzeSmbURI( uri + 6, path, checkFS, count );
	}

#endif

	if ( uri[0] == 'f' && uri[1] == 't' && uri[2] == 'p' && uri[3] == ':' && uri[4] == '/' && uri[5] == '/' )
	{
		return ParzeFtpURI( uri + 6, path, checkFS, count );
	}

#if defined(LIBSSH_EXIST) || defined(LIBSSH2_EXIST)

	if ( uri[0] == 's' && uri[1] == 'f' && uri[2] == 't' && uri[3] == 'p' && uri[4] == ':' && uri[5] == '/' && uri[6] == '/' )
	{
		return ParzeSftpURI( uri + 7, path, checkFS, count );
	}

#endif

	if ( uri[0] == 'f' && uri[1] == 'i' && uri[2] == 'l' && uri[3] == 'e' && uri[4] == ':' && uri[5] == '/' && uri[6] == '/' )
	{
		uri += 6;   //оставляем 1 символ '/'
	}


#ifdef _WIN32
	int c = uri[0];

	if ( c == '\\' || c == '/' )
	{
		if ( uri[1] == '/' || uri[1] == '\\' )
		{
			FSString link = uri + 1;

			if ( !ParzeLink( path, link ) ) { return FSPtr(); }

			if ( path.Count() == 1 )
			{
				FSPtr netFs = new FSWin32Net( 0 );
				path.Set( CS_UTF8, "/" );
				return netFs;
			}

			if ( path.Count() == 2 )
			{
				static unicode_t aa[] = {'\\', '\\', 0};
				carray<wchar_t> name = UnicodeToUtf16( carray_cat<unicode_t>( aa, path.GetItem( 1 )->GetUnicode() ).data() );

				NETRESOURCEW r;
				r.dwScope = RESOURCE_GLOBALNET;
				r.dwType = RESOURCETYPE_ANY;
				r.dwDisplayType = RESOURCEDISPLAYTYPE_GENERIC;
				r.dwUsage = RESOURCEUSAGE_CONTAINER;
				r.lpLocalName = 0;
				r.lpRemoteName = name.data();
				r.lpComment = 0;
				r.lpProvider = 0;

				FSPtr netFs = new FSWin32Net( &r );
				path.Set( CS_UTF8, "/" );
				return netFs;
			}

			return new FSSys( -1 );
		}

		FSString link = uri;

		if ( !ParzeLink( path, link ) ) { return FSPtr(); }

		return ( count > 0 && !checkFS[0].IsNull() ) ? checkFS[0] : FSPtr();
	}

	if ( c >= 'A' && c <= 'Z' ) { c = c - 'A' + 'a'; }

	if ( c < 'a' || c > 'z' || uri[1] != ':' || ( uri[2] != '/' && uri[2] != '\\' ) )
	{
		FSString link = uri;

		if ( !ParzeLink( path, link ) ) { return FSPtr(); }

		return ( count > 0 && !checkFS[0].IsNull() ) ? checkFS[0] : FSPtr();
	}

	FSString link = uri + 2;

	if ( !ParzeLink( path, link ) ) { return FSPtr(); }

	return new FSSys( c - 'a' );
#else
	FSString link = uri;

	if ( !ParzeLink( path, link ) ) { return FSPtr(); }

	if ( uri[0] != '/' )
	{
		return ( count > 0 && !checkFS[0].IsNull() ) ? checkFS[0] : FSPtr();
	}

	return new FSSys(); //systemFSPtr;
#endif
}


FSPtr ParzeCurrentSystemURL( FSPath& path )
{
#ifdef _WIN32

	int drive = 3; //c:
	char buf[0x100];
	UINT l = GetSystemDirectory( buf, sizeof( buf ) );

	if ( l > 0 )
	{
		int c = buf[0];
		drive = ( c >= 'A' && c <= 'Z' ) ? c - 'A' : ( ( c >= 'a' && c <= 'z' ) ? c - 'a' : 3 );
	}

	path.Set( CS_UTF8, "/" );
	return new FSSys( drive );
#else
	int bufSize = 1024;
	carray<sys_char_t> buf( bufSize );

	while ( true )
	{
		if ( getcwd( buf.ptr(), bufSize ) )
		{
			break;
		}

		if ( errno != ERANGE )
		{
			throw_syserr( 0, "Botva" );
		}

		bufSize *= 2;
		buf.resize( bufSize );
	}

	path.Set( sys_charset_id, buf.ptr() );
	return new FSSys(); //systemFSPtr;
#endif
}

