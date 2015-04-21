/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * wcm@linderdaum.com
 */

#pragma once

#include <vector>

#include "vfs.h"

/*
   если uri - относительный, то считается путь относительно  path, и берется первый FS из списка
      если FS нет, то это ошибка и возвращается пустой clPtr<FS>

   path всегда должен быть абсолютным

   возврат: clPtr<FS> и измененный path
      в случае ошибки: clPtr<FS> пустой, path - не определен
*/

clPtr<FS> ParzeURI( const unicode_t* uri, FSPath& path, const std::vector<clPtr<FS>>& checkFS );

clPtr<FS> ParzeCurrentSystemURL( FSPath& path );
