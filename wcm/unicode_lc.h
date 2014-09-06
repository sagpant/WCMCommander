#pragma once

#include "wal/wal_sys_api.h"

/// convert a single UCS-2 character to lowercase
wal::unicode_t UnicodeLC( wal::unicode_t ch );

/// convert a single UCS-2 character to uppercase
wal::unicode_t UnicodeUC( wal::unicode_t ch );
