#pragma once

#include <list>

#include "vfs.h"

struct FSTmpNode
{
	enum NODE_TYPE {NODE_FILE, NODE_DIR};
	NODE_TYPE nodeType;
	FSString name; // how it appears in tmp panel
	FSStat fsStat;
	struct FSTmpNode* parentDir;
	FSPath baseFSPath; // what stands behind file item
	std::list<FSTmpNode> content; // content of a dir node
	FSTmpNode(NODE_TYPE _nodeType, const unicode_t* _name, FSTmpNode* _parentDir)
		: nodeType(_nodeType), name(_name), parentDir(_parentDir){}
	// file type node
	FSTmpNode(FSPath* _baseFSPath, FSStat* _baseFSNodeStat, FSTmpNode* _parentDir, const unicode_t* _name = 0);
	// dir type node
	FSTmpNode(const unicode_t* _name = 0, FSTmpNode* _parentDir = 0);

	FSTmpNode* findByBasePath(FSPath* basePath, bool isRecursive = true);
	FSTmpNode* findByFsPath(FSPath* basePath, int basePathLevel = 0);
	FSTmpNode* findByName(FSString* name, bool isRecursive = true);
	bool Add(FSTmpNode* fsTmpNode);
	bool Remove(FSString* name, bool searchRecursive = true);
	bool Rename(FSString* oldName, FSString* newName)
	{
		FSTmpNode* n = findByName(newName, false);
		if (!n)
			return false;
		else
			n->name = *newName;
	}
};

class FSTmp : public FS
{
	clPtr<FS> baseFS;
	FSTmpNode rootDir;

public:
	static FSPath FSTmp::rootPathName;
	virtual unsigned Flags() { return baseFS->Flags(); };
	virtual bool IsEEXIST(int err){ return err == ERROR_FILE_EXISTS || baseFS->IsEEXIST(err); };
	virtual bool IsENOENT(int err){ return err == ERROR_FILE_NOT_FOUND || baseFS->IsENOENT(err); };
	virtual bool IsEXDEV(int err) { return baseFS->IsEXDEV(err); };
	virtual FSString StrError(int err) { return baseFS->StrError(err); }
	virtual bool Equal(FS* fs){ return fs && fs->Type() == Type(); }

	virtual int OpenRead(FSPath& path, int flags, int* err, FSCInfo* info);
	virtual int OpenCreate(FSPath& path, bool overwrite, int mode, int flags, int* err, FSCInfo* info);
	virtual int Rename(FSPath&  oldpath, FSPath& newpath, int* err, FSCInfo* info);
	virtual int Close(int fd, int* err, FSCInfo* info)
		{	return baseFS->Close(fd, err, info); }
	virtual int Read(int fd, void* buf, int size, int* err, FSCInfo* info)
		{	return baseFS->Read(fd, buf, size, err, info); }
	virtual int Write(int fd, void* buf, int size, int* err, FSCInfo* info)
		{	return baseFS->Write(fd, buf, size, err, info);	}
	virtual int Seek(int fd, SEEK_FILE_MODE mode, seek_t pos, seek_t* pRet, int* err, FSCInfo* info)
		{	return baseFS->Seek( fd, mode, pos, pRet, err, info);	}
	virtual int MkDir(FSPath& path, int mode, int* err, FSCInfo* info);
	virtual int Delete(FSPath& path, int* err, FSCInfo* info);
	virtual int RmDir(FSPath& path, int* err, FSCInfo* info);

	virtual int SetFileTime(FSPath& path, FSTime aTime, FSTime mTime, int* err, FSCInfo* info);
	virtual int ReadDir(FSList* list, FSPath& path, int* err, FSCInfo* info);
	virtual int Stat(FSPath& path, FSStat* st, int* err, FSCInfo* info);
	virtual int FStat(int fd, FSStat* st, int* err, FSCInfo* info)
		{	return baseFS->FStat(fd, st, err, info);	}
	virtual int Symlink(FSPath& path, FSString& str, int* err, FSCInfo* info)
		{	return -1;	}
	virtual int StatVfs(FSPath& path, FSStatVfs* st, int* err, FSCInfo* info);

	virtual int64_t GetFileSystemFreeSpace(FSPath& path, int* err) { return baseFS->GetFileSystemFreeSpace(path, err); }

	virtual FSString Uri(FSPath& path);

	bool AddNode(FSPath& srcPath, FSNode* fsNode, FSPath& destPath);
	FSTmp(clPtr<FS> _baseFS) : FS(TMP), baseFS(_baseFS){}
	virtual ~FSTmp(){}
};
