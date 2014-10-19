/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#pragma once

#include <wal.h>
#include <sys/types.h>

#ifdef _WIN32
#	if !defined( NOMINMAX )
#		define NOMINMAX
#	endif
#	include <windows.h>
#else
#	include <sys/socket.h>
#	include <arpa/inet.h>
#	include <netinet/tcp.h>
#	include <netinet/in.h>
#endif

using namespace wal;

struct Sin
{
	struct sockaddr_in sin;
	Sin( unsigned ip = INADDR_ANY, int port = 0 ) { sin.sin_family = AF_INET; sin.sin_port = htons( port ); sin.sin_addr.s_addr = htonl( ip ); }
	unsigned Ip() const { return ntohl( sin.sin_addr.s_addr ); }
	int Port() const { return ntohs( sin.sin_port ); }
	struct sockaddr* SockAddr() { return ( struct sockaddr* )&sin; }
	int Size() const { return sizeof( struct sockaddr_in ); }
};

#ifdef _WIN32

class TCPSock
{
protected:
	SOCKET sock;
	int SysErr() { return WSAGetLastError(); }

	bool __Select( bool sRead, int timeoutSec = -1 ) //ret false on timeout
	{
		fd_set fs;
		FD_ZERO( &fs );
		FD_SET( sock, &fs );
		struct timeval tv;

		if ( timeoutSec >= 0 ) { tv.tv_sec = timeoutSec; tv.tv_usec = 0; }

		int n = select( /*sock+1*/0, ( sRead ) ? &fs : 0, ( sRead ) ? 0 : &fs, 0, timeoutSec >= 0 ? &tv : 0 );

		if ( n < 0 ) { throw int( SysErr() ); }

		return n != 0 ;
	}

	int __Select2( bool r, bool w, int timeoutSec = -1 ) //ret false on timeout
	{
		fd_set fs;
		FD_ZERO( &fs );
		FD_SET( sock, &fs );
		struct timeval tv;

		if ( timeoutSec >= 0 ) { tv.tv_sec = timeoutSec; tv.tv_usec = 0; }

		int n = select( /*sock+1*/0, r ? &fs : 0, w ? &fs : 0, 0, timeoutSec >= 0 ? &tv : 0 );

		if ( n < 0 ) { throw int( SysErr() ); }

		return n;
	}

public:
	TCPSock(): sock( INVALID_SOCKET ) {}
	SOCKET Id() { return sock; }
	bool IsValid() const { return sock != INVALID_SOCKET;}
	void Create()           { ASSERT(sock == INVALID_SOCKET); if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) { throw int(SysErr()); } }
	void Bind( unsigned i = INADDR_ANY, int p = 0 ) {  Sin sin( i, p );    if ( bind( sock, sin.SockAddr(), sin.Size() ) ) { throw int( SysErr() ); } }

	bool SelectRead( int timeoutSec = -1 )   { return __Select( true, timeoutSec ); }
	bool SelectWrite( int timeoutSec = -1 )  { return __Select( false, timeoutSec ); }
	int Select2( bool r, bool w, int timeoutSec = -1 ) { return __Select2( r, w, timeoutSec ); }

	void Connect( unsigned ip, int port ) { Sin sin( ip, port ); if ( connect( sock, ( struct sockaddr* )&sin, sizeof( sin ) ) ) { throw int( SysErr() ); } }
	void Shutdown()            { if ( sock != INVALID_SOCKET && shutdown( sock, /*SD_RECEIVE*/0 ) ) { throw int( SysErr() ); } }
	void Close( bool aborted = false ) { if ( sock != INVALID_SOCKET ) { if ( closesocket( sock ) && !aborted ) { throw int( SysErr() ); } sock = INVALID_SOCKET; }; }

	void Listen()           { if ( listen( sock, 16 ) ) { throw int( SysErr() ); } }

	int Read( void* buf, int size )    { int bytes = recv( sock, ( char* ) buf, size, 0 ); if ( bytes < 0 ) { throw int( SysErr() ); } return bytes; }
	int Write( void* buf, int size )      { int bytes = send( sock, ( char* ) buf, size, 0 ); if ( bytes <= 0 ) { throw int( SysErr() ); } return bytes; }

	void SetNoDelay( bool yes = true )
	{
		BOOL OPT = yes ? TRUE : FALSE;

		if ( setsockopt( sock, IPPROTO_TCP, TCP_NODELAY , ( const char* )&OPT, sizeof( OPT ) ) )
		{
			throw int( SysErr() );
		}
	}
	void GetSockName( Sin& s )      { int l = sizeof( Sin ); if ( getsockname( sock, s.SockAddr(), &l ) ) { throw int( SysErr() ); } }
	int GetSoError()        { int ret; int l = sizeof( ret ); if ( getsockopt( sock, SOL_SOCKET, SO_ERROR, ( char* )&ret, &l ) ) { throw int( SysErr() ); } return ret; }

	void Accept( TCPSock& retSock, Sin& retSin )
	{
		int size = retSin.Size();
		SOCKET r = accept( sock, retSin.SockAddr(), &size );

		if ( r == INVALID_SOCKET ) { throw int( SysErr() ); }

		retSock.Close();
		retSock.sock = r;
	}

	void SetNonBlock( bool yes = true )
	{
		u_long mode = ( yes ? 1 : 0 );

		if ( ioctlsocket( sock, FIONBIO, &mode ) ) { throw int( SysErr() ); }
	}

	~TCPSock() { if ( sock != INVALID_SOCKET ) { closesocket( sock ); } }
};

#else
class TCPSock
{
protected:
	int sock;
	bool __Select( bool sRead, int timeoutSec = -1 ) //ret false on timeout
	{
		fd_set fs;
		FD_ZERO( &fs );
		FD_SET( sock, &fs );
		struct timeval tv;

		if ( timeoutSec >= 0 ) { tv.tv_sec = timeoutSec; tv.tv_usec = 0; }

		int n = select( sock + 1, ( sRead ) ? &fs : 0, ( sRead ) ? 0 : &fs, 0, timeoutSec >= 0 ? &tv : 0 );

		if ( n < 0 ) { throw int( errno ); }

		return n != 0 ;
	}

	int __Select2( bool r, bool w, int timeoutSec = -1 ) //ret false on timeout
	{
		fd_set fs;
		FD_ZERO( &fs );
		FD_SET( sock, &fs );
		struct timeval tv;

		if ( timeoutSec >= 0 ) { tv.tv_sec = timeoutSec; tv.tv_usec = 0; }

		int n = select( sock + 1, r ? &fs : 0, w ? &fs : 0, 0, timeoutSec >= 0 ? &tv : 0 );

		if ( n < 0 ) { throw int( errno ); }

		return n;
	}

public:
	TCPSock(): sock( -1 ) {}
	int Id() { return sock; }
	bool IsValid() const { return sock > 0; }
	void Create()           { ASSERT( sock < 0 ); if ( ( sock = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 ) { throw int( errno ); } }
	void Bind( unsigned i = INADDR_ANY, int p = 0 ) {  Sin sin( i, p );    if ( bind( sock, sin.SockAddr(), sin.Size() ) ) { throw int( errno ); } }
	bool SelectRead( int timeoutSec = -1 )   { return __Select( true, timeoutSec ); }
	bool SelectWrite( int timeoutSec = -1 )  { return __Select( false, timeoutSec ); }
	int Select2( bool r, bool w, int timeoutSec = -1 ) { return __Select2( r, w, timeoutSec ); }
	void Connect( unsigned ip, int port ) { Sin sin( ip, port ); if ( connect( sock, ( struct sockaddr* )&sin, sizeof( sin ) ) ) { throw int( errno ); } }
	void Shutdown()            { if ( sock >= 0 && shutdown( sock, SHUT_RD ) ) { throw int( errno ); } }
	void Close( bool aborted = false ) { if ( sock >= 0 ) { if ( close( sock ) && !aborted ) { throw int( errno ); } sock = -1; }; }
	void Listen()           { if ( listen( sock, 16 ) ) { throw int( errno ); } }
	int Read( void* buf, int size )    { int bytes = read( sock, buf, size ); if ( bytes < 0 ) { throw int( errno ); } return bytes; }
	int Write( void* buf, int size )      { int bytes = write( sock, buf, size ); if ( bytes <= 0 ) { throw int( errno ); } return bytes; }
	void SetNoDelay( bool yes = true ) { int n = yes ? 1 : 0; if ( setsockopt( sock, IPPROTO_TCP, TCP_NODELAY, &n, sizeof( n ) ) ) { throw int( errno ); } }
	void GetSockName( Sin& s )      { socklen_t l = sizeof( Sin ); if ( getsockname( sock, s.SockAddr(), &l ) ) { throw int( errno ); } }
	int GetSoError()        { int ret; socklen_t l = sizeof( ret ); if ( getsockopt( sock, SOL_SOCKET, SO_ERROR, &ret, &l ) ) { throw int( errno ); } return ret; }

	void Accept( TCPSock& retSock, Sin& retSin )
	{
		socklen_t size = retSin.Size();
		int r = accept( sock, retSin.SockAddr(), &size );

		if ( r < 0 ) { throw int( errno ); }

		retSock.Close();
		retSock.sock = r;
	}

	void SetNonBlock( bool yes = true )
	{
		long flags = fcntl( sock, F_GETFL );

		if ( flags < 0 ) { throw int( errno ); }

		if ( yes ) { flags |= O_NONBLOCK; }
		else { flags &= ~long( O_NONBLOCK ); }

		if ( fcntl( sock, F_SETFL, flags ) ) { throw int( errno ); }
	}

	~TCPSock() { if ( sock >= 0 ) { close( sock ); } }
};

#endif

struct InBuf
{
	int size;
	std::vector<char> data;
	int pos, count;
	InBuf( int bufSize = 1024 * 16 ): size( bufSize ), data( bufSize ), pos( 1 ), count( 1 ) {}
	int Pop( char* s, int n ) { if ( n > count - pos ) { n = count - pos; } if ( n < 0 ) { return 0; } memcpy( s, data.data() + pos, n ); pos += n; return n; }
	void Clear() { pos = 1; count = 1; }
};

struct OutBuf
{
	int size;
	std::vector<char> data;
	int count;
	int Space() { return size - count; }
	OutBuf( int bufSize = 1024 * 16 ): size( bufSize ), data( bufSize ), count( 0 ) {}
	int Push( const char* s, int n ) { if ( n > Space() ) { n = Space(); } if ( !n ) { return 0; } memcpy( data.data() + count, s, n ); count += n; return n; }
	void Clear() { count = 0; }
};

typedef bool ( *CheckStopFunc )( void* param );

#define TIMEOUT_CONNECT (16)
#define TIMEOUT_READ (16)
#define TIMEOUT_WRITE (16)
#define TIMEOUT_ACCEPT (16)

class TCPSyncBufProto
{
	TCPSock sock;

	InBuf inBuf;
	OutBuf outBuf;

	CheckStopFunc stopFunc;
	void* stopParam;

	bool Eof() { return inBuf.count == 0; }

	void SelectRead()
	{
		for ( int t = TIMEOUT_READ; t > 0; t-- )
		{
			if ( sock.SelectRead( 1 ) ) { return; }

			if ( stopFunc && stopFunc( stopParam ) ) { throw int( -2 ); }
		}

		throw int( -3 );
	}

	void SelectWrite()
	{
		for ( int t = TIMEOUT_WRITE; t > 0; t-- )
		{
			if ( sock.SelectWrite( 1 ) ) { return; }

			if ( stopFunc && stopFunc( stopParam ) ) { throw int( -2 ); }
		}

		throw int( -3 );
	}

	void WriteBuf()
	{
		if ( outBuf.count <= 0 ) { return; }

		int pos = 0;

		while ( pos < outBuf.count )
		{
			SelectWrite();
			pos += sock.Write( outBuf.data.data() + pos, outBuf.count - pos );
		}

		outBuf.count = 0;
	}

	void ReadBuf()
	{
		if ( inBuf.pos < inBuf.count || Eof() ) { return; }

		WriteBuf(); //!!!
		SelectRead();
		int bytes = sock.Read( inBuf.data.data(), inBuf.size );
		inBuf.count = bytes;
		inBuf.pos = 0;
	}
public:
	TCPSyncBufProto(): stopFunc( 0 ), stopParam( 0 ) {}

	void GetSockName( Sin& s ) { sock.GetSockName( s ); }

	void Connect( unsigned ip, int port )
	{
		sock.Create();

		try
		{
			sock.Bind();
			sock.SetNonBlock();

			try { sock.Connect( ip, port ); }
			catch ( int e )
			{
#ifdef _WIN32

				if ( e != WSAEWOULDBLOCK ) { throw; }

#else

				if ( e != EINPROGRESS ) { throw; }

#endif
				SelectWrite();
				int r  = sock.GetSoError();

				if ( r ) { throw int( r ); }
			}
		}
		catch ( ... )
		{
			sock.Close();
			throw;
		}
	}

	int GetC() { ReadBuf(); return Eof() ? int( EOF ) : int( inBuf.data[inBuf.pos++] ) & 0xFF; }

	TCPSock& Sock() { return sock; }

	char* ReadLine( char* buf, int size )
	{
		size--;
		int n = 0;
		char* s = buf;
		dbg_printf( "\n" );

		for ( ; size > 0; s++, size--, n++ )
		{
			int c = GetC();

			if ( c >= 32 ) { dbg_printf( "%c", c ); }

			if ( c == '\n' )
			{
				*s = 0;
				return buf;
			}
			else if ( c == EOF )
			{
				*s = 0;
				return n > 0 ? buf : 0;
			}

			*s = c;
		}

		while ( true )
		{
			int c = GetC();

			if ( c == '\n' || c == EOF ) { break; }
		}

		return buf;
	}

	void Flush() { WriteBuf(); }
	void Close( bool aborted = false ) { if ( !aborted ) { Flush(); sock.Shutdown(); };  sock.Close( aborted ); inBuf.Clear(); outBuf.Clear(); }
	void PutC( unicode_t c ) { if ( !outBuf.Space() ) { WriteBuf(); } outBuf.data[outBuf.count++] = char( c & 0xFF ); }
	void WriteStr( const char* s ) { for ( ; *s; s++ ) { PutC( *s ); } }
	void WriteStr( const char* s1, const char* s2 ) { WriteStr( s1 ); WriteStr( s2 ); }
	void WriteStr( const char* s1, const char* s2, const char* s3 ) { WriteStr( s1 ); WriteStr( s2 ); WriteStr( s3 ); }

	void Write( const char* s, int size )
	{
		while ( size > 0 )
		{
			int n = outBuf.Push( s, size );

			if ( !n ) { WriteBuf(); }

			size -= n;
			s += n;
		}
	}

	int Read( char* s, int size )
	{
		int _n = size;

		while ( size > 0 )
		{
			int n = inBuf.Pop( s, size );

			if ( !n )
			{
				if ( Eof() ) { break; }

				ReadBuf();
			}

			size -= n;
			s += n;
		}

		return _n - size;
	}

	void SetStopFunc( CheckStopFunc f, void* data ) { stopFunc = f; stopParam = data; }

	~TCPSyncBufProto() {}
};

bool GetHostIp( const char* utf8, unsigned* ip, int* err );
