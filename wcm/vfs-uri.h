/*
   Copyright (c) by Valery Goryachev (Wal)
*/


#ifndef VFS_URI_H
#define VFS_URI_H

#include "vfs.h"

/*
   если uri - относительный, то считается путь относительно  path, и берется первый FS из списка
      если FS нет, то это ошибка и возвращается пустой clPtr<FS>

   path всегда должен быть абсолютным

   возврат: clPtr<FS> и измененный path
      в случае ошибки: clPtr<FS> пустой, path - не определен
*/

clPtr<FS> ParzeURI( const unicode_t* uri, FSPath& path, clPtr<FS>* checkFS, int count );

clPtr<FS> ParzeCurrentSystemURL( FSPath& path );

//extern clPtr<FS> systemclPtr<FS>;

#endif
