#include "vfs-tmp.h"



static void dbg_prinf_fspath(const char* label, FSPath& path)
{
#ifdef _DEBUG
	dbg_printf("%s:(%d): ", label, path.Count());
	for (int i = 0; i < path.Count(); i++)
	{
		const char* s = path.GetItem(i)->GetUtf8();
		dbg_printf("%s:", s ? s : "null");
	}
	dbg_printf("\n");
#endif
}

#ifdef _WIN32
const char rootStr[] = "\\";
#else
const char rootStr[] = "/";
#endif
static FSString rootFSStr(rootStr);
FSPath FSTmp::rootPathName(rootFSStr);

FSTmpNode::FSTmpNode(FSPath* _baseFSPath, FSStat* _baseFSNodeStat, FSTmpNode* _parentDir, const unicode_t* _name)
: FSTmpNode(NODE_FILE, _name, _parentDir)
{
	baseFSPath = *_baseFSPath;
	fsStat=*_baseFSNodeStat;
	//fsStat.link = _baseFSPath->GetUnicode();
	if (!_name)
		name = _baseFSPath->GetUnicode();
}

FSTmpNode::FSTmpNode(const unicode_t* _name, FSTmpNode* _parentDir)
: FSTmpNode(NODE_DIR, _name, _parentDir)
{
	fsStat.mode |= S_IFDIR;
	fsStat.mtime = FSTime(FSTime::TIME_CURRENT);
}

FSTmpNode* FSTmpNode::findByBasePath(FSPath* basePath, bool isRecursive)
{
	// first, try all file nodes in current dir
	for (std::list<FSTmpNode>::iterator it = content.begin(); it != content.end(); ++it)
	{
		if ((*it).nodeType == FSTmpNode::NODE_FILE)
		{
			if ((*it).baseFSPath.Equals(basePath))
				return &(*it);
		} 
	}
	// only if not found in current dir, try inside subdirs
	if (isRecursive)
	{
		for (std::list<FSTmpNode>::iterator it = content.begin(); it != content.end(); ++it)
		{
			if ((*it).nodeType == FSTmpNode::NODE_DIR)
			{
				FSTmpNode* n = (*it).findByBasePath(basePath, isRecursive);
				if (n)
					return n;
			}
		}
	}
	return 0;
}

FSTmpNode* FSTmpNode::findByFsPath(FSPath* fsPath, int fsPathLevel)
{
	FSString* curName = fsPath->GetItem(fsPathLevel);
	if (parentDir == 0 && fsPathLevel == fsPath->Count()-1 && (curName == 0 || curName->GetUnicode() == 0 || curName->GetUnicode()[0] == 0)) // request to find root node, and we are root
		return this;

	if (name.Cmp(*curName) == 0)
	{
		if (fsPath->Count() <= fsPathLevel)
		{   // exact match
			return this;
		}
		else
		{
			FSString* childName = fsPath->GetItem(fsPathLevel + 1);
			FSTmpNode* n = findByName(childName, false);
			if (fsPath->Count() <= fsPathLevel + 2) // no further recursion
				return n;
			else if (n->nodeType == NODE_DIR) // recurse into subdir
				return n->findByFsPath(fsPath, fsPathLevel + 1);
			else
				return 0;
		}
	}
	else
		return false;
}


FSTmpNode* FSTmpNode::findByName(FSString* name, bool isRecursive)
{
	dbg_printf("FSTmpNodeDir::findByName name=%s\n",name->GetUtf8());
	// first, try all nodes in current	
	for (std::list<FSTmpNode>::iterator it = content.begin(); it != content.end(); ++it)
	{
		dbg_printf("FSTmpNodeDir::findByName *it.name=%s\n", (*it).name.GetUtf8());

		if ((*it).name.Cmp(*name)==0)
			return &(*it);
	}
	// only if not found in current dir, try inside subdirs
	if (isRecursive)
	{
		for (std::list<FSTmpNode>::iterator it = content.begin(); it != content.end(); ++it)
		{
			if ((*it).nodeType == FSTmpNode::NODE_DIR)
			{
				FSTmpNode* n = (*it).findByName(name, isRecursive);
				if (n)
					return n;
			}
		}
	}
	return 0;
}

bool FSTmpNode::Remove(FSString* name, bool searchRecursive)
{
	// first, try all nodes in current dir
	for (std::list<FSTmpNode>::iterator it = content.begin(); it != content.end(); ++it)
	{
		if ((*it).name.Cmp(*name) == 0)
		{
			content.erase(it);
			return true;
		}
	}
	// only if not found in current dir, try inside subdirs
	if (searchRecursive)
	{
		for (std::list<FSTmpNode>::iterator it = content.begin(); it != content.end(); ++it)
		{
			if ((*it).nodeType == FSTmpNode::NODE_DIR)
			{
				bool ret = (*it).Remove(name, searchRecursive);
				if (ret)
					return true;
			}
		}
	}
	return false;
}

bool FSTmpNode::Add(FSTmpNode* fsTmpNode)
{
	if (findByName(&fsTmpNode->name))
		return false;
	fsTmpNode->parentDir = this;
	content.push_back(*fsTmpNode);
	return true;
}

int FSTmp::ReadDir(FSList* list, FSPath& path, int* err, FSCInfo* info)
{
	list->Clear();
	FSTmpNode* n = rootDir.findByFsPath(&path);
	if (!n || n->nodeType != FSTmpNode::NODE_DIR)
	{
		return FS::SetError(err, ERROR_FILE_NOT_FOUND);
	}

	for (std::list<FSTmpNode>::iterator it = n->content.begin(); it != n->content.end(); ++it)
	{
		clPtr<FSNode> pNode = new FSNode();

		pNode->name=(std::string("") + (*it).name.GetUtf8()).c_str();
		pNode->st = (*it).fsStat;

		list->Append(pNode);
	}
	return 0;
}

int FSTmp::Stat(FSPath& path, FSStat* st, int* err, FSCInfo* info)
{
	dbg_prinf_fspath("FSTmp::Stat ", path);
	FSTmpNode* n = rootDir.findByFsPath(&path);
	if (!n)
	{
		return FS::SetError(err, ERROR_FILE_NOT_FOUND);
	}
	*st = n->fsStat;
	return FS::SetError(err, 0);
}

int FSTmp::StatVfs(FSPath& path, FSStatVfs* st, int* err, FSCInfo* info)
{	
	dbg_prinf_fspath("FSTmp::StatVfs ", path);
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
	dbg_prinf_fspath("FSTmp::OpenRead ", path);
	FSTmpNode* n = rootDir.findByFsPath(&path);
	if (n == 0 || n->nodeType != FSTmpNode::NODE_FILE)
	{
		return FS::SetError(err, ERROR_FILE_NOT_FOUND);
	}
	else
	{
		dbg_prinf_fspath("FSTmp::OpenRead baseFSPath=", n->baseFSPath);
		return baseFS->OpenRead(n->baseFSPath, flags, err, info);
	}
}
	

int FSTmp::OpenCreate(FSPath& path, bool overwrite, int mode, int flags, int* err, FSCInfo* info)
{
	// can not create orphan file in TMP panel
	return FS::SetError(err, ERROR_FILE_NOT_FOUND);
}

int FSTmp::Rename(FSPath&  oldpath, FSPath& newpath, int* err, FSCInfo* info)
{
	FSTmpNode* n = rootDir.findByName(oldpath.GetItem(oldpath.Count() - 1));
	if (n == 0)
	{
		return FS::SetError(err, ERROR_FILE_NOT_FOUND);
	}
	else
	{
		if (n->nodeType == FSTmpNode::NODE_FILE)
		{
			int ret = baseFS->Rename(n->baseFSPath, newpath, err, info);
			if (ret != 0)
				return ret;
			n->name = newpath.GetUnicode();
			return SetError(err, 0);
		}
		else
		{// XXX ??? add case when new and old path are in different dirs
			((FSTmpNode*)(n))->name = *newpath.GetItem(newpath.Count() - 1);
			return SetError(err, 0);
		}
	}
}

int FSTmp::Delete(FSPath& path, int* err, FSCInfo* info)
{
	FSString* dName = path.GetItem(path.Count() - 1);
	FSTmpNode* n = rootDir.findByName(dName);
	if (n == 0)
	{
		return FS::SetError(err, ERROR_FILE_NOT_FOUND);;
	}
	else
	{
		/*
		if (n->nodeType == FSTmpNode::NODE_FILE)
		{ // remove file at base FS
			int ret = baseFS->Delete(n->baseFSPath, err, info);
			if (ret != 0)
				return ret;
		}
		*/
		// remove from tmpfs list
		for (std::list<FSTmpNode>::iterator it = n->parentDir->content.begin(); it != n->parentDir->content.end(); ++it)
		{ 
			if ((*it).name.Cmp(*dName) == 0)
			{
				n->parentDir->content.erase(it);
				return FS::SetError(err, 0);
			}
		}
		return FS::SetError(err, ERROR_FILE_NOT_FOUND);;
	}
}

int FSTmp::RmDir(FSPath& path, int* err, FSCInfo* info)
{
	FSTmpNode* n = rootDir.findByFsPath(&path);
	FSString* dirName = path.GetItem(path.Count() - 1);

	for (std::list<FSTmpNode>::iterator it = n->parentDir->content.begin(); it != n->parentDir->content.end(); ++it)
	{
		if ((*it).name.Cmp(*dirName) == 0)
		{
			n->parentDir->content.erase(it);
			return FS::SetError(err, 0);
		}
	}
	return FS::SetError(err, ERROR_FILE_NOT_FOUND);
}

int FSTmp::SetFileTime(FSPath& path, FSTime aTime, FSTime mTime, int* err, FSCInfo* info)
{
	FSTmpNode* fsTemp = rootDir.findByName(path.GetItem(path.Count() - 1));
	if (fsTemp == 0)
	{
		return FS::SetError(err, ERROR_FILE_NOT_FOUND);
	}
	else
	{
		fsTemp->fsStat.mtime = mTime;

		if (fsTemp->nodeType == FSTmpNode::NODE_FILE)
			return baseFS->SetFileTime(fsTemp->baseFSPath, aTime, mTime, err, info);
		else
			return FS::SetError(err, 0);
			
	}
}

FSString FSTmp::Uri(FSPath& path) 
{ 
	std::string uri(std::string("tmp://") + baseFS->Uri(path).GetUtf8());
	if (uri.back() != DIR_SPLITTER)
		uri += DIR_SPLITTER;
	return FSString(uri.c_str());
};

bool FSTmp::AddNode(FSPath& srcPath, FSNode* fsNode, FSPath& destPath)
{
	FSPath parentDir = destPath;
	parentDir.Pop();
	dbg_printf("FSTmp::AddNode srcPath.Count()=%d, parentDir.Count()=%d\n", srcPath.Count(), parentDir.Count());
	dbg_prinf_fspath("FSTmp::AddNode srcPath=", srcPath);
	dbg_prinf_fspath("FSTmp::AddNode parentDir=", parentDir);

	FSTmpNode* dn = rootDir.findByFsPath(&parentDir);
	if (!dn || dn->nodeType != FSTmpNode::NODE_DIR)
	{
		return false;
	}

	return dn->Add(&FSTmpNode(&srcPath, &fsNode->st, dn));
}

int FSTmp::MkDir(FSPath& path, int mode, int* err, FSCInfo* info)
{
	FSPath parentPath;
	parentPath.Copy(path, path.Count() - 1);
	FSTmpNode* parent = rootDir.findByFsPath(&parentPath);
	if (!parent)
		return SetError(err, ERROR_FILE_NOT_FOUND);
	FSTmpNode* parentDir = parent;
	parentDir->Add(&FSTmpNode(path.GetItem(path.Count() - 1)->GetUnicode(), parentDir));
	return SetError(err, 0);
}
