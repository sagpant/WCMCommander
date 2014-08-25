/*
	Copyright (c) by Valery Goryachev (Wal)
*/

#include "wal.h"

namespace wal {

	static void def_thread_error_func(int err, const char *msg, const char *file, int *line)
	{
		if (!msg) msg = "";
		fprintf(stderr, "THREAD ERROR: %s\n", msg);
	}

	void (*thread_error_func)(int err, const char *msg, const char *file, int *line) = def_thread_error_func; 


	carray<unicode_t> new_unicode_str(const unicode_t *s)
	{
		if ( !s ) return carray<unicode_t>();
		const unicode_t *p;
		for (p = s; *p; ) p++;
		int len = p-s;
		carray<unicode_t> r(len + 1);
		if (len) memcpy(r.ptr(), s, len* sizeof(unicode_t));
		r[len]=0;
		return r;
	}

	carray<sys_char_t> new_sys_str(const sys_char_t *s)
	{
		if ( !s ) return carray<sys_char_t>();
		int len = sys_strlen(s);
		carray<sys_char_t> r(len+1);
		if (len) memcpy(r.ptr(), s, len* sizeof(sys_char_t));
		r[len]=0;
		return r;
	}

	carray<char> new_char_str(const char *s)
	{
		if ( !s ) return carray<char>();
		int len = strlen(s);
		carray<char> r(len+1);
		
		if (len)
			memcpy(r.ptr(), s, len* sizeof(char));
			
		r[len]=0;
		return r;
	}

	
	carray<sys_char_t> utf8_to_sys(const char *s)
	{
		if ( !s ) return carray<sys_char_t>();
		int symbolCount = utf8_symbol_count(s);
		carray<unicode_t> unicodeBuf(symbolCount + 1);
		utf8_to_unicode(unicodeBuf.ptr(), s);

		int sys_len = sys_string_buffer_len(unicodeBuf.ptr(), symbolCount);
		carray<sys_char_t> sysBuf(sys_len+1);
		unicode_to_sys(sysBuf.ptr(), unicodeBuf.ptr(), symbolCount);
		return sysBuf;
	};
        

	carray<char> sys_to_utf8(const sys_char_t *s)
	{
		if ( !s ) return carray<char>();
		int symbolCount=sys_symbol_count(s);
		carray<unicode_t> unicodeBuf(symbolCount + 1);
		sys_to_unicode(unicodeBuf.ptr(), s);
		int utf8Len = utf8_string_buffer_len(unicodeBuf.ptr(), symbolCount);
		carray<char> utf8Buf(utf8Len+1);
		unicode_to_utf8(utf8Buf.ptr(), unicodeBuf.ptr(), symbolCount);
		return utf8Buf;
	}


	carray<unicode_t> utf8_to_unicode(const char *s)
	{
		if ( !s ) return carray<unicode_t>();
		int symbolCount = utf8_symbol_count(s);
		carray<unicode_t> unicodeBuf(symbolCount + 1);
		utf8_to_unicode(unicodeBuf.ptr(), s);
		return unicodeBuf;
	}
	
	carray<char> unicode_to_utf8(const unicode_t *u)
	{
		if ( !u ) return carray<char>();
		int size = utf8_string_buffer_len(u);
		carray<char> s(size + 1);
		unicode_to_utf8(s.ptr(), u);
		return s;
	}
	
	
////////////  system File implementation

void File::Throw(const sys_char_t *name)
{
	if (!name) name = _fileName.ptr();
	static const char noname[] = "<NULL>";
	throw_syserr(0, "'%s'", name ? sys_to_utf8(name).ptr() : noname);
}

void File::Throw(){ Throw(0); }

}; //namespace wal
