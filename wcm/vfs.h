/*
	Copyright (c) by Valery Goryachev (Wal)
*/


#ifndef NCVFS_H
#define NCVFS_H

#include <wal.h>
#include "ncdialogs.h"

#include "vfspath.h"
#include "strconfig.h"

#ifdef _WIN32
class FSTime {
	enum{ FILETIME_OK=1, TIME_T_OK=2 };
	char flags;
	FILETIME ft;
	time_t tt;
public:
	FSTime():flags(0){}
	FSTime(time_t t):flags(TIME_T_OK), tt(t){}
	FSTime(FILETIME t):flags(FILETIME_OK), ft(t){}
	time_t GetTimeT();
	FILETIME GetFileTime();
	void SetTimeT(time_t val){ tt = val; flags = TIME_T_OK; }
	void SetFileTime(FILETIME val){ ft = val; flags = FILETIME_OK; }

	operator time_t(){ return GetTimeT(); }
	operator FILETIME(){ return GetFileTime(); }
	FSTime& operator = (time_t t){ SetTimeT(t); return *this; }
	FSTime& operator = (FILETIME ft){ SetFileTime(ft); return *this; }
};
#else
typedef time_t FSTime;
#endif

#ifdef _WIN32
#define S_IFMT	0170000	//bitmask for the file type bitfields
#define S_IFSOCK	0140000	//socket
#define S_IFLNK	0120000	//symbolic link
#define S_IFREG	0100000	//regular file
#define S_IFBLK	0060000	//block device
#define S_IFDIR	0040000	//directory
#define S_IFCHR	0020000	//character device
#define S_IFIFO	0010000	//fifo
#define S_ISUID	0004000	//set UID bit
#define S_ISGID	0002000	//set GID bit (see below)
#define S_ISVTX	0001000	//sticky bit (see below)
#define S_IRWXU	 00700	//mask for file owner permissions
#define S_IRUSR	00400	//owner has read permission
#define S_IWUSR	00200	//owner has write permission
#define S_IXUSR	00100	//owner has execute permission
#define S_IRWXG	00070	//mask for group permissions
#define S_IRGRP	00040	//group has read permission
#define S_IWGRP	00020	//group has write permission
#define S_IXGRP	00010	//group has execute permission
#define S_IRWXO	00007	//mask for permissions for others (not in group)
#define S_IROTH	00004	//others have read permission
#define S_IWOTH	00002	//others have write permisson
#define S_IXOTH	00001	//others have execute permission
#endif

struct FSStat {
	FSString link;

#ifdef _WIN32
	DWORD dwFileAttributes;
#endif
	int mode;
	int64 size;
	FSTime mtime;
	int uid;
	int gid;
	
	dev_t dev;
	ino_t ino;
	
	FSStat():
#ifdef _WIN32
		dwFileAttributes(0),
#endif
		mode(0), size(0), mtime(0), uid(-1), gid(-1), dev(0), ino(0)
		{}

	FSStat(const FSStat &a): mode(a.mode), size(a.size), mtime(a.mtime), uid(a.uid), gid(a.gid), dev(a.dev), ino(a.ino)
		{ link.Copy(a.link); }
	FSStat& operator = (const FSStat &a)
		{  link.Copy(a.link); mode=a.mode; size=a.size; mtime=a.mtime; uid=a.uid; gid=a.gid; dev=a.dev; ino=a.ino; return *this;}
	
	bool IsLnk() const { return !link.IsNull(); }
	bool IsReg() const { return (mode & S_IFMT) == S_IFREG; }
	bool IsDir() const { return (mode & S_IFMT) == S_IFDIR; }
	bool IsExe() const { return (mode & (S_IXUSR | S_IXGRP | S_IXOTH)) !=0; }
	bool IsBad() const { return (mode & S_IFMT) == 0; }
	void SetBad(){ mode =0; }
	
	unicode_t* GetMTimeStr(unicode_t buf[64]);
	unicode_t* GetModeStr(unicode_t buf[64]);
	unicode_t* GetPrintableSizeStr(unicode_t buf[64]);
	
	~FSStat(){};
};

#ifdef _WIN32
class W32NetRes { 
	struct Node {
		int size;
		NETRESOURCEW rs;
	};
	unsigned char *data;
	void Clear(){ if (data) { delete [] data; data = 0; } };
	static unsigned char *Copy(const unsigned char *);
public:
	W32NetRes():data(0){}
	void Set(NETRESOURCEW *p);

	W32NetRes(NETRESOURCEW *p):data(0){Set(p);}
	W32NetRes(const W32NetRes &a):data(Copy(a.data)){}
	W32NetRes& operator = (const W32NetRes &a){ Clear(); data = Copy(a.data); return *this; }
	NETRESOURCEW* Get() volatile { return (data) ? &(((Node*)data)->rs) : 0; }
	~W32NetRes(){ if (data) delete [] data; }
};
#endif


struct FSNode {
	enum EXTTYPES {	SERVER = 1, WORKGROUP, FILESHARE };

	#ifdef _WIN32
		W32NetRes _w32NetRes;
	#endif 
	
	bool isSelected;
	int extType;
	FSStat st;
	FSString name;
	FSNode *next;
	FSNode *originNode;
	
	FSNode():isSelected(false), extType(0), next(0), originNode(0) {}
	FSNode(const FSNode&a): isSelected(a.isSelected), extType(a.extType), st(a.st), next(0), originNode(0) { name.Copy(a.name); }
	FSNode& operator = (const FSNode&a)
		{ isSelected = a.isSelected; extType = a.extType; st = a.st; next=0; name.Copy(a.name); originNode = a.originNode; return *this;}
	
	FSString& Name(){ return name; } 		
	int64 Size() const { return st.size; }
	bool IsDir() const { return st.IsDir(); }
	bool IsLnk() const { return st.IsLnk(); }
	bool IsReg() const { return st.IsReg(); }
#ifdef _WIN32
	bool IsExe();
#else
	bool IsExe() const { return st.IsExe(); }
#endif	
	bool IsBad() const { return st.IsBad(); }

#ifdef _WIN32
	bool IsHidden() { return (st.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) !=0; }
#else
	bool IsHidden() { return name.GetUnicode()[0] == '.'; }
#endif	
	bool IsSelected() const { return isSelected; }
	void SetSelected(){ isSelected = true; }
	void ClearSelected(){ isSelected = false; }
	const unicode_t *GetUnicodeName(){ return name.GetUnicode(); }
	
	int GetUID() const { return st.uid; }
	int GetGID() const { return st.gid; }

	int CmpByName(FSNode &a, bool case_sensitive){ return case_sensitive ? name.Cmp(a.name) : name.CmpNoCase(a.name); }
	int CmpByExt(FSNode &a, bool case_sensitive);

	~FSNode();
};


class FSList {
	FSNode *first, *last;
	int count;
public:
	FSList():first(0),last(0),count(0){};
	int Count() const { return count; };
	
	FSNode* First() { return first; }
	
	void Append(cptr<FSNode>);
	void Clear();
	
	wal::carray<FSNode*> GetArray();
	
	wal::carray<FSNode*> GetFilteredArray(bool showHidden, int *pCount);
	
	void CopyFrom(const FSList &a, bool onlySelected = false);
	void CopyOne(FSNode *node);
	
	~FSList(){ Clear(); }
	
	static void SortByName	(FSNode **buf, int count, bool asc, bool case_sensitive);
	static void SortByExt	(FSNode **buf, int count, bool asc, bool case_sensitive);
	static void SortBySize	(FSNode **buf, int count, bool asc);
	static void SortByMTime	(FSNode **buf, int count, bool asc);
	
CLASS_COPY_PROTECTION(FSList);
};

struct FSSmbParam {

	volatile char server[0x100]; //all utf8
	volatile char domain[0x100];
	volatile char user[0x100];
	volatile char pass[0x100]; 
	volatile bool isSet;
	
	FSSmbParam():isSet(0){ server[0]=domain[0]=user[0]=pass[0]=0;  }
	
	void SetServer(const char *s){ 
		volatile char *p = server; 
		int n = sizeof(server)-1;
		for (;*s && n>0; s++, p++, n--)
			*p = *s;
		*p=0;
	}

	void GetConf(StrConfig &conf)
	{
		conf.Set("SERVER", const_cast<char*>(server));
		conf.Set("DOMAIN", const_cast<char*>(domain));
		conf.Set("USER", const_cast<char*>(user));
	}

	static void _copy(char *dest, const char *src, int destSize)
	{
		int n = destSize;
		for ( ;*src && n>1; src++, dest++, n--) *dest = *src;
		*dest = 0;
	}


	void SetConf(StrConfig &conf)
	{
		const char *s = conf.GetStrVal("SERVER");
		if (s) _copy(const_cast<char*>(server), s, sizeof(server));

		s = conf.GetStrVal("DOMAIN");
		if (s) _copy(const_cast<char*>(domain), s, sizeof(domain));

		s = conf.GetStrVal("USER");
		if (s) _copy(const_cast<char*>(user), s, sizeof(user));
	}
};

template <int SIZE = 128> class UFString {
	volatile unicode_t data[SIZE];
public:
	void Set(const unicode_t *strZ){ volatile unicode_t *s=data; int n = SIZE-1; for ( ;*strZ && n>0; s++, strZ++, n--) *s=*strZ; *s=0; }
	UFString(){ data[0]=0; }
	UFString(const unicode_t *strZ){ Set(strZ); }
	UFString& operator = (const unicode_t *strZ){ Set(strZ); return *this;}
	const unicode_t* Data(){ return const_cast<unicode_t*>(data); }
	~UFString(){}
};


struct FSFtpParam {
	UFString<> server;
	volatile int port;
	volatile int charset;
	volatile bool anonymous;
	UFString<> user;
	UFString<> pass;
	volatile bool passive;
	volatile bool isSet;
	FSFtpParam():port(21), 
#ifdef _WIN32
		charset(CS_UTF8),
#else
		charset(sys_charset_id), 
#endif
		anonymous(true), isSet(false), passive(true){}

	void GetConf(StrConfig &conf)
	{
		conf.Set("SERVER", unicode_to_utf8(server.Data()).ptr());
		conf.Set("USER", unicode_to_utf8(user.Data()).ptr());
		conf.Set("PORT", port);
		conf.Set("ANONYMOUS", anonymous ? 1 : 0);
		conf.Set("PASSIVE", passive ? 1 : 0);

		conf.Set("CHARSET", charset_table.NameById(charset));
	}


	void SetConf(StrConfig &conf)
	{
		const char *s = conf.GetStrVal("SERVER");
		if (s) server.Set(utf8_to_unicode(s).ptr());

		s = conf.GetStrVal("USER");
		if (s) user.Set(utf8_to_unicode(s).ptr());

		int n = conf.GetIntVal("PORT");
		if (n>0) port = n;

		n = conf.GetIntVal("ANONYMOUS");
		anonymous = n==0 ? false : true;

		n = conf.GetIntVal("PASSIVE");
		passive = n==0 ? false : true;

		charset = charset_table.IdByName( conf.GetStrVal("CHARSET") );

		isSet = anonymous;
	}


	~FSFtpParam(){}
};


struct FSSftpParam {
	UFString<> server;
	volatile int port;
	volatile int charset;
	UFString<> user;
//	UFString<> pass;
	volatile bool isSet;
	FSSftpParam():port(22), 
#ifdef _WIN32
		charset(CS_UTF8),
#else
		charset(sys_charset_id), 
#endif
		isSet(false){}

	void GetConf(StrConfig &conf)
	{
		conf.Set("SERVER", unicode_to_utf8(server.Data()).ptr());
		conf.Set("USER", unicode_to_utf8(user.Data()).ptr());
		conf.Set("PORT", port);
		conf.Set("CHARSET", charset_table.NameById(charset));
	}

	void SetConf(StrConfig &conf)
	{
		const char *s = conf.GetStrVal("SERVER");
		if (s) server.Set(utf8_to_unicode(s).ptr());

		s = conf.GetStrVal("USER");
		if (s) user.Set(utf8_to_unicode(s).ptr());

		int n = conf.GetIntVal("PORT");
		if (n>0) port = n;

		charset = charset_table.IdByName( conf.GetStrVal("CHARSET") );
	}

	~FSSftpParam(){}
};

struct FSPromptData {
	volatile bool visible;
	UFString<32> prompt;
};

class FSCInfo {
public:
	FSCInfo(){}
	virtual bool Stopped();
	virtual bool SmbLogon(FSSmbParam *a);
	virtual bool FtpLogon(FSFtpParam *a);
	virtual bool Prompt(const unicode_t *header, const unicode_t *message, FSPromptData *p, int count);
	virtual ~FSCInfo();
};

class FSCSimpleInfo: public FSCInfo {
public:
	Mutex mutex;
	bool stopped;
	FSCSimpleInfo():stopped(false){}
	void Reset(){ MutexLock lock(&mutex); stopped = false; }
	void SetStop(){ MutexLock lock(&mutex); stopped = true; }
	virtual bool Stopped();
	virtual ~FSCSimpleInfo();
};


/*
	все фанкции, возвращающие int возвращают 0 при успехе  -1 при ошибке и -2 при StopEvent
	одна FS (один и тот же объект) может в один период времени мспользоваться только одним потоком
	
	id файла - int
*/


class FS {
	friend class FSPtr;	
	volatile int _copyCounter;
public:
	enum TYPES { SYSTEM = 0, SFTP = 1, SAMBA = 2, FTP = 3, WIN32NET = 4 };
	enum FLAGS { HAVE_READ = 1, HAVE_WRITE = 2, HAVE_SYMLINK = 4, HAVE_SEEK = 8 };
	enum OPENFLAGS { SHARE_READ = 1, SHARE_WRITE = 2 };
private:
	int _type;
public:
	FS(int t):_copyCounter(0), _type(t) { }
	int Type() const { return _type; }
	
	static void SetError(int *p, int err){ if (p) *p = err; }
	
	virtual unsigned Flags() 		= 0;
	virtual bool IsEEXIST(int err)		= 0;
	virtual bool IsENOENT(int err)		= 0;
	virtual bool IsEXDEV(int err)		= 0;
	bool IsNORENAME(int err) { return IsEXDEV(err); };
	virtual FSString StrError(int err)	= 0;	//строка ощибки с указанным идентификатором
	virtual bool Equal(FS *fs) = 0; //должна отрабатывать null (как false)
	
	virtual int OpenRead	(FSPath &path, int flags, int *err, FSCInfo *info);
	virtual int OpenCreate	(FSPath &path, bool overwrite, int mode, int flags, int *err, FSCInfo *info);
	virtual int Close	(int fd, int *err, FSCInfo *info);
	virtual int Read	(int fd, void *buf, int size, int *err, FSCInfo *info);
	virtual int Write	(int fd, void *buf, int size, int *err, FSCInfo *info);
	virtual int Seek	(int fd, SEEK_FILE_MODE mode, seek_t pos, seek_t *pRet,  int *err, FSCInfo *info);
	virtual int Rename	(FSPath  &oldpath, FSPath &newpath, int *err, FSCInfo *info);
	virtual int MkDir	(FSPath &path, int mode, int *err,  FSCInfo *info);
	virtual int Delete	(FSPath &path, int *err, FSCInfo *info);
	virtual int RmDir	(FSPath &path, int *err, FSCInfo *info);
	virtual int SetFileTime	(FSPath &path, FSTime aTime, FSTime mTime, int *err, FSCInfo *info);
	virtual int ReadDir	(FSList *list, FSPath &path,  int *err,FSCInfo *info);
	virtual int Stat(FSPath &path, FSStat *st, int *err, FSCInfo *info);
	virtual int FStat(int fd, FSStat *st, int *err, FSCInfo *info);
	virtual int Symlink	(FSPath &path, FSString &str, int *err, FSCInfo *info);
	virtual int64 GetFileSystemFreeSpace(FSPath &path, int *err);
	
	virtual FSString Uri(FSPath &path)							= 0;
	
	virtual unicode_t* GetUserName(int user, unicode_t buf[64]);
	virtual unicode_t* GetGroupName(int group, unicode_t buf[64]);
	
	virtual ~FS();
CLASS_COPY_PROTECTION(FS);
};

class FSPtr {
	FS * volatile data;
	void Copy(FS *p);
public:
	FSPtr():data(0){}
	FSPtr(FS *fs):data(0){ Copy(fs); }
	FSPtr(const FSPtr &p): data(0){ Copy(p.data); }
	FS* operator -> (){ return data; }
	FS* Ptr(){ return data; }
	FSPtr& operator = (FS *fs){ Copy(fs); return *this; }
	FSPtr& operator = (const FSPtr &p){ Copy(p.data); return *this; }
	bool IsNull() const { return data == 0; }
	~FSPtr(){ Copy(0); }
};


#ifdef _WIN32
class FSSysHandles {
	Mutex mutex;
	enum { SIZE=32 };
	struct Node {
		volatile bool busy;
		volatile HANDLE handle;
	};
	Node table[SIZE];
public:
	FSSysHandles(){ for (int i = 0; i<SIZE; i++) table[i].busy = false; }
	int New(){ 
		MutexLock lock(&mutex); 
		for (int i =0; i<SIZE; i++) 
			if (!table[i].busy) { table[i].handle = INVALID_HANDLE_VALUE; table[i].busy = true; return i; }
		return -1;
	};
	HANDLE* Handle(int n) 
	{
		MutexLock lock(&mutex);
		if (n>=0 && n<SIZE && table[n].busy)
		{
			return const_cast<HANDLE*>(&(table[n].handle));
		}
		return 0;
	}
	void Free(int n) 
	{
		MutexLock lock(&mutex);
		if (n>=0 && n<SIZE && table[n].busy)
		{
			table[n].busy = false;
			table[n].handle =  INVALID_HANDLE_VALUE;
		}
	}
};
#endif


class FSSys: public FS {
#ifdef _WIN32
	volatile int _drive;
	FSSysHandles handles;
public:
	FSSys():FS(FS::SYSTEM){}
#endif
public:
#ifdef _WIN32
	//0-'A'...'Z', -1 - network
	explicit FSSys(int drive):FS(FS::SYSTEM), _drive(drive){}
	int Drive() const { return _drive; }
#else
	FSSys():FS(FS::SYSTEM){}
#endif
	virtual unsigned Flags();
	virtual bool IsEEXIST(int err);
	virtual bool IsENOENT(int err);
	virtual bool IsEXDEV(int err);
	virtual FSString StrError(int err);
	virtual bool Equal(FS *fs);
	
	virtual int OpenRead	(FSPath &path, int flags, int *err, FSCInfo *info);
	virtual int OpenCreate	(FSPath &path, bool overwrite, int mode, int flags, int *err, FSCInfo *info);
	virtual int Close	(int fd, int *err, FSCInfo *info);
	virtual int Read	(int fd, void *buf, int size, int *err, FSCInfo *info);
	virtual int Write	(int fd, void *buf, int size, int *err, FSCInfo *info);
	virtual int Seek	(int fd, SEEK_FILE_MODE mode, seek_t pos, seek_t *pRet,  int *err, FSCInfo *info);
	virtual int Rename	(FSPath  &oldpath, FSPath &newpath, int *err,  FSCInfo *info);
	virtual int MkDir	(FSPath &path, int mode, int *err,  FSCInfo *info);
	virtual int Delete	(FSPath &path, int *err, FSCInfo *info);
	virtual int RmDir	(FSPath &path, int *err, FSCInfo *info);
	virtual int SetFileTime	(FSPath &path, FSTime aTime, FSTime mTime, int *err, FSCInfo *info);
	virtual int ReadDir	(FSList *list, FSPath &path, int *err, FSCInfo *info);
	virtual int Stat	(FSPath &path, FSStat *st, int *err, FSCInfo *info);
	virtual int FStat(int fd, FSStat *st, int *err, FSCInfo *info);
	virtual int Symlink	(FSPath &path, FSString &str, int *err, FSCInfo *info);
	virtual FSString Uri(FSPath &path);
	virtual int64 GetFileSystemFreeSpace(FSPath &path, int *err);
	
	virtual unicode_t* GetUserName(int user, unicode_t buf[64]);
	virtual unicode_t* GetGroupName(int group, unicode_t buf[64]);

	virtual ~FSSys();
};



#ifdef _WIN32

class FSWin32Net: public FS {
	volatile W32NetRes  _res;
public:
	enum { ERRNOSUPPORT = -1000 };
	FSWin32Net(NETRESOURCEW *p):FS(FS::WIN32NET),_res(p){};
	virtual unsigned Flags();
	virtual bool IsEEXIST(int err);
	virtual bool IsENOENT(int err);
	virtual bool IsEXDEV(int err);
	virtual FSString StrError(int err);
	virtual bool Equal(FS *fs);
	virtual int OpenRead	(FSPath &path, int flags, int *err, FSCInfo *info);
	virtual int OpenCreate	(FSPath &path, bool overwrite, int mode, int flags, int *err, FSCInfo *info);
	virtual int Close	(int fd, int *err, FSCInfo *info);
	virtual int Read	(int fd, void *buf, int size, int *err, FSCInfo *info);
	virtual int Write	(int fd, void *buf, int size, int *err, FSCInfo *info);
	virtual int Seek	(int fd, SEEK_FILE_MODE mode, seek_t pos, seek_t *pRet,  int *err, FSCInfo *info);
	virtual int Rename	(FSPath  &oldpath, FSPath &newpath, int *err,  FSCInfo *info);
	virtual int MkDir	(FSPath &path, int mode, int *err,  FSCInfo *info);
	virtual int Delete	(FSPath &path, int *err, FSCInfo *info);
	virtual int RmDir	(FSPath &path, int *err, FSCInfo *info);
	virtual int SetFileTime	(FSPath &path, FSTime aTime, FSTime mTime, int *err, FSCInfo *info);
	virtual int ReadDir	(FSList *list, FSPath &path, int *err, FSCInfo *info);
	virtual int Stat	(FSPath &path, FSStat *st, int *err, FSCInfo *info);
	virtual int Symlink	(FSPath &path, FSString &str, int *err, FSCInfo *info);
	virtual FSString Uri(FSPath &path);
	virtual ~FSWin32Net();
};

#endif


//path - должен быть абсолютным (если нет - то вернет false)
extern bool ParzeLink(FSPath &path, FSString &link);

//inline FSPtr GetSysFS(){ return FSPtr(new FSSys()); }


#endif
