/*
	Copyright (c) by Valery Goryachev (Wal)
*/


#ifndef VFS_URI_H
#define VFS_URI_H

#include "vfs.h"

/*
	если uri - относительный, то считается путь относительно  path, и берется первый FS из списка
		если FS нет, то это ошибка и возвращается пустой FSPtr
		
	path всегда должен быть абсолютным
	
	возврат: FSPtr и измененный path
		в случае ошибки: FSPtr пустой, path - не определен
*/

FSPtr ParzeURI(const unicode_t *uri, FSPath &path, FSPtr *checkFS, int count);

FSPtr ParzeCurrentSystemURL(FSPath &path);

//extern FSPtr systemFSPtr;

#endif
