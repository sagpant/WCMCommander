/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * walcommander@linderdaum.com
 */

#pragma once

#include "libconf.h"

#ifdef LIBSMBCLIENT_EXIST

#include "vfs.h"


class FSSmb : public FS
{
	mutable Mutex mutex;
	FSSmbParam _param;
public:
	FSSmb( FSSmbParam* param = 0 );

	virtual unsigned Flags();
	virtual bool IsEEXIST( int err );
	virtual bool IsENOENT( int err );
	virtual bool IsEXDEV( int err );
	virtual FSString StrError( int err );

	virtual bool Equal( FS* fs );

	virtual int OpenRead ( FSPath& path, int flags, int* err, FSCInfo* info );
	virtual int OpenCreate  ( FSPath& path, bool overwrite, int mode, int flags, int* err, FSCInfo* info );
	virtual int Close ( int fd, int* err, FSCInfo* info );
	virtual int Read  ( int fd, void* buf, int size, int* err, FSCInfo* info );
	virtual int Write ( int fd, void* buf, int size, int* err, FSCInfo* info );
	virtual int Seek  ( int fd, SEEK_FILE_MODE mode, seek_t pos, seek_t* pRet,  int* err, FSCInfo* info );
	virtual int Rename   ( FSPath&  oldpath, FSPath& newpath, int* err,  FSCInfo* info );
	virtual int MkDir ( FSPath& path, int mode, int* err,  FSCInfo* info );
	virtual int Delete   ( FSPath& path, int* err, FSCInfo* info );
	virtual int RmDir ( FSPath& path, int* err, FSCInfo* info );
	virtual int SetFileTime ( FSPath& path, FSTime cTime, FSTime aTime, FSTime mTime, int* err, FSCInfo* info ) override;
	virtual int ReadDir  ( FSList* list, FSPath& path, int* err, FSCInfo* info );
	virtual int Stat  ( FSPath& path, FSStat* st, int* err, FSCInfo* info );
	virtual int FStat ( int fd, FSStat* st, int* err, FSCInfo* info );
	virtual int Symlink  ( FSPath& path, FSString& str, int* err, FSCInfo* info );
	virtual int StatVfs( FSPath& path, FSStatVfs* st, int* err, FSCInfo* info );

	virtual FSString Uri( FSPath& path );
	virtual ~FSSmb();

	FSSmbParam GetParamValue() const { MutexLock lock( &mutex ); return _param; }
};

#endif
