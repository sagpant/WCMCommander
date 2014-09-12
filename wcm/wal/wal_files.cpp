/*
   Copyright (c) by Valery Goryachev (Wal)
*/


#ifdef _WIN32
#	if !defined( NOMINMAX )
#		define NOMINMAX
#	endif
#	include <windows.h>
#else
#	include <sys/types.h>
#	include <sys/stat.h>
#	include <fcntl.h>
#	include <unistd.h>
#endif

#include "wal.h"

namespace wal
{

	file_t file_open( const sys_char_t* filename,  int open_flag )
	{
#ifdef _WIN32
		DWORD diseredAccess = 0;
		DWORD shareMode = 0;
		DWORD creationDisposition = 0;

		if ( open_flag & FOPEN_READ ) { diseredAccess |= GENERIC_READ; }

		if ( open_flag & FOPEN_WRITE ) { diseredAccess |= GENERIC_WRITE; }


		if ( open_flag & FOPEN_CREATE )
		{
			creationDisposition = ( ( open_flag & FOPEN_TRUNC ) ? CREATE_ALWAYS : OPEN_ALWAYS );
		}
		else
		{
			creationDisposition = ( ( open_flag & FOPEN_TRUNC ) ? TRUNCATE_EXISTING : OPEN_EXISTING );
		}

		HANDLE f = CreateFileW( filename, diseredAccess, FILE_SHARE_READ, 0, creationDisposition,
		                        ( open_flag & FOPEN_SYNC ) ? FILE_FLAG_WRITE_THROUGH : 0,
		                        0 );
		return f;
#else

		int flag = 0;

		if ( open_flag & FOPEN_CREATE ) { flag |= O_CREAT; }

		if ( open_flag & FOPEN_SYNC ) { flag |= O_SYNC; }

		if ( open_flag & FOPEN_TRUNC ) { flag |= O_TRUNC; }

		if ( ( open_flag & FOPEN_RW ) == FOPEN_RW )
		{
			flag |= O_RDWR;
		}
		else
		{
			if ( open_flag & FOPEN_READ ) { flag |= O_RDONLY; }

			if ( open_flag & FOPEN_WRITE ) { flag |= O_WRONLY; }
		}

		return open( filename, flag, 0600 );

#endif
	}

	file_t file_open_utf8( const char* filename, int open_flag )
	{
#ifdef _WIN32
		//...
#else

		if ( sys_charset_id == CS_UTF8 )
		{
			return file_open( filename, open_flag );
		}

#endif

		SysStringStruct buf( filename );
		const sys_char_t* s = buf.get();

		if ( s )
		{
			return file_open( s, open_flag );
		}

		// out of memory (sys error is set by SysStringStruct)

		return FILE_NULL;
	}

}; //namespace wal

