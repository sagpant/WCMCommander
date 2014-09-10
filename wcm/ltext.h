#pragma once

#include "wal.h"

bool LTextLoad( wal::sys_char_t* );
const char* LText( const char* index );
const char* LText( const char* index, const char* def );
#define _LT LText
