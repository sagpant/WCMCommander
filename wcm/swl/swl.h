/*
   Copyright (c) by Valery Goryachev (Wal)
*/

#pragma once

#define _CRT_SECURE_NO_WARNINGS
#include "../wal/wal.h"
#include "swl_internal.h"
#include "swl_wincore.h"
#include "swl_winbase.h"

#ifndef EMPTY_OPER
#	define EMPTY_OPER
#endif

#if defined(_MSC_VER)
#  pragma warning(disable : 4355) // 'this' : used in base member initializer list
#endif

inline bool IsSpace( int c ) { return c > 0 && c <= 32; }
inline bool IsAlpha( int c ) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
inline bool IsAlphaHex( int c ) { return ( c >= 'a' && c <= 'f' ) || ( c >= 'A' && c <= 'F' ); }
inline bool IsDigit( int c ) { return c >= '0' && c <= '9'; }
