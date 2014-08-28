/*
   Copyright (c) by Valery Goryachev (Wal)
*/

#ifndef _WIN32
#ifndef SHELL_H
#define SHELL_H

#include "wal.h"

using namespace wal;

class Shell
{
	pid_t pid;
	int in, out;

	void Stop();
	void Run();
	carray<char> slaveName;
public:
	Shell( const char* slave );

	pid_t Exec( const char* cmd );
	int Wait( pid_t pid, int* pStatus );
	int CD( const char* path );

	~Shell();
};

#endif
#endif
