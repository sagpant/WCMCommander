#include "vfs-tmp.h"


FSPath FSTmp::rootPathName({ DIR_SPLITTER, 0 });

FSTmpNode* FSTmp::findByBasePath(FSPath* basePath)
{
	for (std::list<FSTmpNode>::iterator it = tmpList.begin(); it != tmpList.end(); ++it)
	{
		if ((*it).baseFSPath.Equals(basePath))
			return &(*it);
	}
	return 0;
}

FSTmpNode* FSTmp::findByTmpFSString(FSString* tmpFSString)
{
	for (std::list<FSTmpNode>::iterator it = tmpList.begin(); it != tmpList.end(); ++it)
	{
		if ((*it).tmpFSString.Cmp(*tmpFSString)==0)
			return &(*it);
	}
	return 0;
}


bool FSTmp::AddNode(FSPath& srcPath, FSNode* fsNode)
{
	// XXX check dups
	if (findByBasePath(&srcPath))
		return false;

	tmpList.push_back(FSTmpNode(srcPath.GetUnicode(), &srcPath, &fsNode->st));
	return true;
}

int FSTmp::ReadDir(FSList* list, FSPath& path, int* err, FSCInfo* info)
{
	list->Clear();
	for (std::list<FSTmpNode>::iterator it = tmpList.begin(); it != tmpList.end(); ++it)
	{
		clPtr<FSNode> pNode = new FSNode();

		pNode->name=it->tmpFSString;
		pNode->st = it->baseFSNodeStat;

		list->Append(pNode);
	}
	return 0;
}


int FSTmp::RemoveFromLocalList(FSString* tmpFSString)
{
	for (std::list<FSTmpNode>::iterator it = tmpList.begin(); it != tmpList.end(); ++it)
	{
		if ((*it).tmpFSString.Cmp(*tmpFSString)==0)
		{
			tmpList.erase(it);
			return 0;
		}
	};
	return -1;
}

int FSTmp::Stat(FSPath& path, FSStat* st, int* err, FSCInfo* info)
{
	if (rootPathName.Equals(&path))
	{
		*st = FSStat();
		st->mode |= S_IFDIR;// temporary panel is a directory
		return 0;
	}
	else
		return baseFS->Stat(path, st, err, info);
}

int FSTmp::StatVfs(FSPath& path, FSStatVfs* st, int* err, FSCInfo* info)
{	
	if (rootPathName.Equals(&path))
	{
		*st = FSStatVfs();
		return 0;
	}
	else
		return baseFS->StatVfs(path, st, err, info);
}

int FSTmp::OpenRead(FSPath& path, int flags, int* err, FSCInfo* info)
{
	FSTmpNode* fsTemp = findByTmpFSString(path.GetItem(path.Count() - 1));
	if (fsTemp == 0)
	{
		*err = -1; // XX put there NOT FOUND code
		return *err;
	}
	else
	{
		return baseFS->OpenRead(fsTemp->baseFSPath, flags, err, info);
	}
}
	

int FSTmp::OpenCreate(FSPath& path, bool overwrite, int mode, int flags, int* err, FSCInfo* info)
{
	// can not create orphan file in TMP panel
	*err = -1; // XX put there NOT FOUND code
	return *err;
}

int FSTmp::Rename(FSPath&  oldpath, FSPath& newpath, int* err, FSCInfo* info)
{
	FSTmpNode* fsTemp = findByTmpFSString(oldpath.GetItem(oldpath.Count() - 1));
	if (fsTemp == 0)
	{
		*err = -1; // XX put there NOT FOUND code
		return *err;
	}
	else
	{
		int ret = baseFS->Rename(fsTemp->baseFSPath, newpath, err, info);
		if (ret == 0)
			RemoveFromLocalList(&fsTemp->tmpFSString);
		return ret;
	}
}

int FSTmp::Delete(FSPath& path, int* err, FSCInfo* info)
{
	FSTmpNode* fsTemp = findByTmpFSString(path.GetItem(path.Count() - 1));
	if (fsTemp == 0)
	{
		*err = -1; // XX put there NOT FOUND code
		return *err;
	}
	else
	{
		int ret = baseFS->Delete(fsTemp->baseFSPath, err, info);
		if (ret == 0)
			RemoveFromLocalList(&fsTemp->tmpFSString);
		return ret;
	}
}

int FSTmp::RmDir(FSPath& path, int* err, FSCInfo* info)
{
	FSTmpNode* fsTemp = findByTmpFSString(path.GetItem(path.Count() - 1));
	if (fsTemp == 0)
	{
		*err = -1; // XX put there NOT FOUND code
		return *err;
	}
	else
	{
		int ret = baseFS->RmDir(fsTemp->baseFSPath, err, info);
		if (ret == 0)
			RemoveFromLocalList(&fsTemp->tmpFSString);
		return ret;
	}
	return -1;
}

int FSTmp::SetFileTime(FSPath& path, FSTime aTime, FSTime mTime, int* err, FSCInfo* info)
{
	return baseFS->SetFileTime(path, aTime, mTime, err, info);
	FSTmpNode* fsTemp = findByTmpFSString(path.GetItem(path.Count() - 1));
	if (fsTemp == 0)
	{
		*err = -1; // XX put there NOT FOUND code
		return *err;
	}
	else
	{
		int ret = baseFS->SetFileTime(fsTemp->baseFSPath, aTime, mTime, err, info);
		if (ret == 0)
			RemoveFromLocalList(&fsTemp->tmpFSString);
		return ret;
	}
}

FSString FSTmp::Uri(FSPath& path) 
{ 
	return FSString((std::string("tmp://") + baseFS->Uri(path).GetUtf8() + DIR_SPLITTER ).c_str());
};

