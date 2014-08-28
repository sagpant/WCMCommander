/*
   Copyright (c) by Valery Goryachev (Wal)
*/

#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <termios.h>
#include <sys/ioctl.h>

#include "shell.h"

/*
   протокол внутреннего шела - синхронный (запрос - ответ)
   запрос:
      char - комманда
      char - количество параметров 0 - 127
      параметры:
         строки strz
   ответ:
      char - код результата не 0 если ошибка
      char - количество параметров 0 - 127
      параметры:
         строки strz
*/


enum CMDS
{
   CMD_EXEC = 1,
   CMD_WAIT = 2,
   CMD_CD = 3
};


static void ShellProc( int in, int out );


Shell::Shell( const char* slave )
	:  pid( -1 ),
	   in( -1 ),
	   out( -1 ),
	   slaveName( new_char_str( slave ) )
{
	Run();
}


void Shell::Stop()
{
	if ( pid > 0 )
	{
		kill( pid, 9 );
		int status = 0;
		waitpid( pid, &status, 0 );
		pid = -1;
	}

	if ( in >= 0 ) { close( in ); in = -1; }

	if ( out >= 0 ) { close( out ); out = -1; }
}

void Shell::Run()
{
	Stop();

	int pipe1[2] = { -1, -1};
	int pipe2[2] = { -1, -1};

	if ( pipe( pipe1 ) || pipe( pipe2 ) )
	{
		printf( "can`t create pipe\n" );
	}

	pid_t p = fork();

	if ( !p )
	{
		close( pipe1[0] );
		close( pipe2[1] );

		setenv( "TERM", "xterm", 1 );

		pid_t sid = setsid();

		if ( sid < 0 ) { printf( "setsid error\n" ); }

		int slaveFd = open( slaveName.ptr(), O_RDWR );

		if ( slaveFd < 0 ) { printf( "can`t open slave terminal file '%s'\n", slaveName.ptr() ); }

		//недоделано
		struct termios attr;

		if ( !tcgetattr( slaveFd, &attr ) )
		{
			attr.c_lflag |= ECHO | ISIG | ICANON;
			attr.c_cc[VINTR] = 3;
			tcsetattr( slaveFd, TCSAFLUSH, &attr );
		}


		dup2( slaveFd, 0 );
		dup2( slaveFd, 1 );
		dup2( slaveFd, 2 );
		close( slaveFd );

		//сделать терминал - контролирующим терминалом
		if ( ioctl( 0, TIOCSCTTY, 0 ) ) { printf( "ioctl(0, TIOCSCTTY, 0) error\n" ); }

		ShellProc( pipe2[0], pipe1[1] );
		exit( 1 );
	}

	in = pipe1[0];
	out = pipe2[1];

	close( pipe1[1] );
	close( pipe2[0] );

	fcntl( in, F_SETFD, long( FD_CLOEXEC ) );
	fcntl( out, F_SETFD, long( FD_CLOEXEC ) );

	pid = p;

//printf(" run shell (pid=%i)\n", p);

}


Shell::~Shell()
{
	Stop();
}


static void ReadPipe( int fd, void* p, int size ) //throw int
{
	int r = read( fd, p, size );

	if ( r < 0 ) { throw int( 1 ); }

	if ( r != size ) { throw int( 2 ); }
}

static void WritePipe( int fd, void* p, int size ) //throw int
{
	int r = write( fd, p, size );

	if ( r < 0 ) { throw int( 1 ); }

	if ( r != size ) { throw int( 2 ); }
}

inline char ReadPipeChar( int fd )
{
	char c;
	ReadPipe( fd, &c, sizeof( c ) );
	return c;
}

inline void WritePipeChar( int fd, char c )
{
	WritePipe( fd, &c, sizeof( c ) );
}

static void WritePipeStr( int fd, const char* s )
{
	while ( true )
	{
		WritePipeChar( fd, *s );

		if ( !*s ) { break; }

		s++;
	}
}

static carray<char> ReadPipeStr( int fd )
{
	ccollect<char, 0x100> p;
	char c;

	while ( true )
	{
		c = ReadPipeChar( fd );
		p.append( c );

		if ( !c ) { break; }
	}

	return p.grab();
}

static bool ReadCmd( int fd, int* pCmd, ccollect<carray<char> >& params )
{
	try
	{
		*pCmd = ReadPipeChar( fd );
		params.clear();
		int count = ReadPipeChar( fd );

		for ( int i = 0; i < count; i++ ) //!!!
		{
			params.append( ReadPipeStr( fd ) );
		}
	}
	catch ( int n )
	{
		return false;
	}

	return true;
}

static bool WriteCmd( int fd, int cmd, ccollect<char*>& list )
{
	try
	{
		WritePipeChar( fd, cmd );
		WritePipeChar( fd, list.count() );

		for ( int i = 0; i < list.count(); i++ ) { WritePipeStr( fd, list[i] ); }
	}
	catch ( int n )
	{
		return false;
	}

	return true;
}

static bool WriteCmd( int fd, int cmd, ccollect<carray<char> >& list )
{
	try
	{
		WritePipeChar( fd, cmd );
		WritePipeChar( fd, list.count() );

		for ( int i = 0; i < list.count(); i++ ) { WritePipeStr( fd, list[i].ptr() ); }
	}
	catch ( int n )
	{
		return false;
	}

	return true;
}


static bool WriteCmd( int fd, int cmd, const char* s )
{
	try
	{
		WritePipeChar( fd, cmd );
		WritePipeChar( fd, 1 );
		WritePipeStr( fd, s );
	}
	catch ( int n )
	{
		return false;
	}

	return true;
}

inline void AddInt( ccollect<carray<char> >& list, int n )
{
	char buf[64];
	sprintf( buf, "%i", n );
	list.append( new_char_str( buf ) );
}


static void ShellProc( int in, int out )
{
	fcntl( in, F_SETFD, long( FD_CLOEXEC ) );
	fcntl( out, F_SETFD, long( FD_CLOEXEC ) );

	signal( SIGINT, SIG_IGN );

	while ( true )
	{
		ccollect<carray<char> > pList;
		int cmd = 0;

		if ( !ReadCmd( in, &cmd, pList ) ) { exit( 1 ); }

		switch ( cmd )
		{
			case CMD_EXEC:
			{
				pid_t pid = fork();

				if ( !pid )
				{
					signal( SIGINT, SIG_DFL );

					static char shell[] = "/bin/sh";

					if ( pList.count() )
					{
						const char* params[] = {shell, "-c", pList[0].ptr(), NULL};
						execv( shell, ( char** ) params );
						printf( "error execute %s\n", shell );
					}
					else
					{
						printf( "internal err (no shall paremeters)\n" );
					}

					exit( 1 );
				}

				char buf[64];
				sprintf( buf, "%i", int( pid ) );

				if ( !WriteCmd( out, 0, buf ) )
				{
					exit( 1 );
				}
			}
			break;

			case CMD_WAIT:
			{
				if ( pList.count() <= 0 ) { printf( "intermnal error 1\n" ); exit( 1 ); }

				int status = 0;
				int ret = waitpid( atoi( pList[0].ptr() ), &status, 0 );

				pList.clear();
				AddInt( pList, ret );
				AddInt( pList, status );
				AddInt( pList, errno );

				if ( !WriteCmd( out, 0, pList ) ) { exit( 1 ); }
			}
			break;

			case CMD_CD:
			{
				if ( pList.count() <= 0 ) { printf( "intermnal error 2\n" ); exit( 1 ); }

				int ret = chdir( pList[0].ptr() );
				pList.clear();
				AddInt( pList, ret );
				AddInt( pList, errno );

				if ( !WriteCmd( out, 0, pList ) ) { exit( 1 ); }

			}
			break;

			default:
				if ( !WriteCmd( out, 1, "unknown internal shell command" ) ) { exit( 1 ); }

				printf( "internal err (unknown shell command)\n" );
				break;
		};
	}
}


pid_t Shell::Exec( const char* cmd )
{
	if ( !WriteCmd( out, CMD_EXEC, cmd ) ) { Run(); return -1; }

	ccollect<carray<char> > list;
	int r;

	if ( !ReadCmd( in, &r, list ) ) { Run(); return -1; }

	if ( list.count() <= 0 ) { Run(); return -1; }

	return atoi( list[0].ptr() );
}

int Shell::Wait( pid_t pid, int* pStatus )
{
	char buf[64];
	sprintf( buf, "%i", pid );

	if ( !WriteCmd( out, CMD_WAIT, buf ) ) { Run(); return -1; }

	ccollect<carray<char> > list;
	int r;

	if ( !ReadCmd( in, &r, list ) ) { Run(); return -1; }

	if ( list.count() <= 0 ) { Run(); return -1; }

	return atoi( list[0].ptr() );
}

int Shell::CD( const char* path )
{
	if ( !WriteCmd( out, CMD_CD, path ) ) { Run(); return -1; }

	ccollect<carray<char> > list;
	int r;

	if ( !ReadCmd( in, &r, list ) ) { Run(); return -1; }

	if ( list.count() <= 0 ) { Run(); return -1; }

	return atoi( list[0].ptr() );
}


