/*
	Copyright (c) by Valery Goryachev (Wal)
*/

#ifndef _____WAL_H
#define _____WAL_H

#include "wal_tmpls.h"
#include "wal_sys_api.h"

#if defined(_MSC_VER)
#	pragma warning(disable : 4200) // nonstandard extension used : zero-sized array in struct/union
#endif

#include <string>

namespace wal {

#ifdef _WIN32
#define snprintf _snprintf
#endif

typedef std::basic_string<unicode_t> unicode_string;
typedef std::basic_string<sys_char_t> sys_string;

carray<unicode_t> new_unicode_str(const unicode_t *);
carray<sys_char_t> new_sys_str(const sys_char_t *);
carray<char> new_char_str(const char *);

carray<sys_char_t> utf8_to_sys(const char *s);
carray<char> sys_to_utf8(const sys_char_t *s);

carray<unicode_t> sys_to_unicode_array(const sys_char_t *s, int size = -1);
carray<sys_char_t> unicode_to_sys_array(const unicode_t *s, int size = -1);

carray<sys_char_t> sys_error_str(int err);
carray<unicode_t> sys_error_unicode(int err);

carray<char> sys_error_utf8(int err);

carray<unicode_t> utf8_to_unicode(const char *s);
carray<char> unicode_to_utf8(const unicode_t *u);

///////////////////  Exceptions ///////////////////////////////////////////////

//all exceptions created by "new" operator (except oom)
//all must deleted by destroy member function
//messages must be in utf8

class cexception {
public:
	cexception(){};
	void destroy();
	virtual const char *message();
	virtual ~cexception();
};

class coom: public cexception {
public:
	coom(){};
	virtual const char *message();
	virtual ~coom();
};

class cmsg: public cexception {
	wal::carray<char> _msg;
public:
	cmsg(const char *s);
	virtual const char *message();
	virtual ~cmsg();
};

class csyserr: public cmsg {
public:
	int code;
	csyserr(int _code, const char *msg);
	virtual ~csyserr();
};

class cstop_exception: public cexception {
public:
	cstop_exception(){};
	virtual const char *message();
	virtual ~cstop_exception();
};

void throw_oom();
void throw_msg(const char *format, ...);
void throw_syserr(int err=0, const char *format = 0, ...);
void throw_stop();

void init_exceptions();

//////////////////////// ///////////////////////////////////////////////////////////

#define CLASS_COPY_PROTECTION(a) private: a(const a&){}; a& operator = (const a&){};

////////////////////////  C++ thread wrappers //////////////////////////////////////
extern void (*thread_error_func)(int err, const char *msg, const char *file, int *line); //by default - abort process

inline void check_thread_error(int err, const char *s = 0){ if (err) thread_error_func(err, s, 0, 0); }

#ifndef WAL_THREAD_CHECK
	#define WAL_THREAD_CHECK(a) if (!(a)) abort();
#endif

class MutexLock;
class Cond;

class Mutex {
	friend class MutexLock;
	friend class Cond;
	mutex_t _mutex;
#ifdef _WIN32
	volatile bool _static;
#endif
public:
	Mutex(bool stat = false);
	void Lock();
	void Unlock();
	~Mutex();
};

class MutexLock {
	Mutex *_pmutex;
	bool _locked;
public:
	MutexLock(Mutex *m, bool lock = true);
	void Lock();
	void Unlock();
	~MutexLock();
};

class Cond {
	cond_t _cond;
public:
	Cond();
	void Wait(Mutex *mutex);
	void Signal();
	void Broadcast();
	~Cond();
};



////////////////// C++ system file wrapper (bo buffered io)

class File {
	file_t _fd;
	carray<sys_char_t> _fileName;
	void Throw(const sys_char_t *name);
	void Throw();
public:
	File() : _fd(FILE_NULL){ }
	void Open(const sys_char_t *name, int open_flag = FOPEN_READ);
	void Close();
	int Read(void * buf, int size);
	int Write(void * buf, int size);
	seek_t Seek(seek_t distance, SEEK_FILE_MODE method = FSEEK_BEGIN );
	~File(); //закрывает без эксепшенов (лучше сначала вызывать Close) 
};
	
////////////////// C++ thread wrappers implementation //////////////////

//
/// Mutex
//
inline Mutex::Mutex(bool stat)
{
#ifdef _WIN32
	_static = stat;
#endif
	check_thread_error(mutex_create(&_mutex), "mutex create");
}

inline void Mutex::Lock()
{
	check_thread_error(mutex_lock(&_mutex), "mutex lock");
}

inline void Mutex::Unlock()
{
	check_thread_error(mutex_unlock(&_mutex), "mutex unlock");
}

inline Mutex::~Mutex()
{
#ifdef _WIN32
	if (!_static)
	check_thread_error(mutex_delete(&_mutex), "mutex destroy"); 
#else 
	check_thread_error(mutex_delete(&_mutex), "mutex destroy"); 
#endif
}


//
// MutexLock
//

inline MutexLock::MutexLock(Mutex *m, bool lock)
:_pmutex(m), _locked(lock)
{
	if (lock) 
		_pmutex->Lock();
}

inline void MutexLock::Lock()
{
	if (!_locked) 
	{
		_pmutex->Lock();
		_locked = true;
	}
}

inline void MutexLock::Unlock()
{
	if (_locked) 
	{
		_pmutex->Unlock();
		_locked = false;
	}
}

inline MutexLock::~MutexLock()
{
	Unlock();
}

//
//  Cond implementation
//

inline Cond::Cond()
{
	check_thread_error(cond_create(&_cond),"cond create");
}

inline void Cond::Wait(Mutex *mutex)
{
	check_thread_error(cond_wait(&_cond, &(mutex->_mutex)), "cond wait");
}

inline void Cond::Signal()
{
	check_thread_error(cond_signal(&_cond), "cond signal"); 
}

inline void Cond::Broadcast()
{
	check_thread_error(cond_broadcast(&_cond), "cond broadcast"); 
}

inline Cond::~Cond()
{
	check_thread_error(cond_delete(&_cond),"cond destroy");
}




///////////////////////////////// system File implamantation

inline void File::Open(const sys_char_t *name, int open_flag)
{
	Close();
	_fd = file_open(name, open_flag);
	if (!valid_file(_fd)) { Throw(name); }
	_fileName = new_sys_str(name);
}

inline void File::Close()
{
	if (!valid_file(_fd)) return;
	if (file_close(_fd)) Throw();
	_fileName.clear();
}

inline int File::Read(void * buf, int size)
{
	int ret = file_read(_fd, buf, size);
	if (ret<0) Throw();
	return ret;
}

inline int File::Write(void * buf, int size)
{
	int ret = file_write(_fd, buf, size);
	if (ret<0) Throw();
	return ret;
}

inline seek_t File::Seek(seek_t distance, SEEK_FILE_MODE method)
{
	seek_t ret = file_seek(_fd, distance, method);
	if (ret<0) Throw();
	return ret;
}
	
inline File::~File(){	if (_fd != FILE_NULL) file_close(_fd); }





//////////////////////////////////////////////////////////////////////////////////
inline carray<unicode_t> sys_to_unicode_array(const sys_char_t *s, int size)
{
	int l = sys_symbol_count(s, size);
	carray<unicode_t> p(l+1);
	if (l>0) sys_to_unicode(p.ptr(), s, size);
	p[l]=0;
	return p;
}

inline carray<sys_char_t> unicode_to_sys_array(const unicode_t *s, int size)
{
	int l = sys_string_buffer_len(s, size);
	carray<sys_char_t> p(l+1);
	if (l > 0) unicode_to_sys(p.ptr(), s, size);
	p[l]=0;
	return p;
}

inline carray<sys_char_t> sys_error_str(int err)
{
	sys_char_t buf[0x100];
	return new_sys_str(sys_error_str(err, buf, sizeof(buf)/sizeof(sys_char_t)));
}


inline carray<unicode_t> sys_error_unicode(int err)
{
	sys_char_t buf[0x100];
	return sys_to_unicode_array(sys_error_str(err, buf, sizeof(buf)/sizeof(sys_char_t)));
}

inline carray<char> sys_error_utf8(int err)
{
	sys_char_t buf[0x100];
	return sys_to_utf8(sys_error_str(err, buf, sizeof(buf)/sizeof(sys_char_t)));
}

}; //namespace wal

#endif
