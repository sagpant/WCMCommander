/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * wcm@linderdaum.com
 */

#include "plugin-archive.h"
#include "vfs.h"

#include "panel.h"
#include "string-util.h"

#include <list>
#include <algorithm>

static const char g_ArchPluginId[] = "ArchPlugin";


#ifdef _WIN32
#	define FSARCH_ERROR_FILE_EXISTS ERROR_FILE_EXISTS
#	define FSARCH_ERROR_FILE_NOT_FOUND ERROR_FILE_NOT_FOUND
#else
#	define FSARCH_ERROR_FILE_EXISTS EEXIST
#	define FSARCH_ERROR_FILE_NOT_FOUND ENOENT
#endif // _WIN32


struct FSArchNode
{
	FSString name; // how it appears in panel
	FSStat fsStat;
	
	struct FSArchNode* parentDir;
	std::list<FSArchNode> content; // content of a dir node
	
	FSArchNode();
	FSArchNode( const char* Name, FSStat& Stat )
	: name( Name ), fsStat( Stat ), parentDir( nullptr ) {}
	
	FSArchNode* findByFsPath( FSPath& basePath, int basePathLevel = 0 );
	FSArchNode* findByName( const FSString& name, bool isRecursive = true );
	
	bool IsDir() const { return fsStat.IsDir(); }
	
	FSArchNode* Add( const FSArchNode& fsArchNode );
	bool Remove( const FSString& name, bool searchRecursive = true );
	bool Rename( const FSString& oldName, const FSString& newName )
	{
		FSArchNode* n = findByName( newName, false );
		if ( !n )
		{
			return false;
		}
		
		n->name = newName;
		return true;
	}
};


class FSArch : public FS
{
	//CLASS_COPY_PROTECTION( FSArch );
private:
	FSArch(const FSArch&) : FS( ARCH ) {};
	FSArch& operator = (const FSArch&) { return *this; };
	
	FSArchNode rootDir;
	
public:
	FSArch( FSArchNode& RootDir ) : FS( ARCH ), rootDir( RootDir ) {}
	virtual ~FSArch() {}
	
	//
	// FS interface
	//
	virtual bool IsPersistent() override { return false; }
	
	virtual unsigned Flags() override { return 0; }
	
	virtual bool IsEEXIST( int err ) override { return err == FSARCH_ERROR_FILE_EXISTS; }
	
	virtual bool IsENOENT( int err ) override { return err == FSARCH_ERROR_FILE_NOT_FOUND; }
	
	virtual bool IsEXDEV( int err ) override { return false; }
	
	virtual FSString StrError( int err ) override { return FSString( "" ); }
	
	virtual bool Equal( FS* fs ) override { return fs && fs->Type() == Type(); }
	
	virtual int OpenRead( FSPath& path, int flags, int* err, FSCInfo* info ) override
	{
		return FS::SetError( err, FSARCH_ERROR_FILE_NOT_FOUND );
	}
	
	virtual int OpenCreate( FSPath& path, bool overwrite, int mode, int flags, int* err, FSCInfo* info ) override
	{
		return FS::SetError( err, FSARCH_ERROR_FILE_NOT_FOUND );
	}
	
	virtual int Rename( FSPath&  oldpath, FSPath& newpath, int* err, FSCInfo* info ) override
	{
		return FS::SetError( err, FSARCH_ERROR_FILE_NOT_FOUND );
	}
	
	virtual int Close( int fd, int* err, FSCInfo* info ) override
	{
		return FS::SetError( err, FSARCH_ERROR_FILE_NOT_FOUND );
	}
	
	virtual int Read( int fd, void* buf, int size, int* err, FSCInfo* info ) override
	{
		return FS::SetError( err, FSARCH_ERROR_FILE_NOT_FOUND );
	}
	
	virtual int Write( int fd, void* buf, int size, int* err, FSCInfo* info ) override
	{
		return FS::SetError( err, FSARCH_ERROR_FILE_NOT_FOUND );
	}
	
	virtual int Seek( int fd, SEEK_FILE_MODE mode, seek_t pos, seek_t* pRet, int* err, FSCInfo* info ) override
	{
		return FS::SetError( err, FSARCH_ERROR_FILE_NOT_FOUND );
	}
	
	virtual int MkDir( FSPath& path, int mode, int* err, FSCInfo* info ) override
	{
		return FS::SetError( err, FSARCH_ERROR_FILE_NOT_FOUND );
	}
	
	virtual int Delete( FSPath& path, int* err, FSCInfo* info ) override
	{
		return FS::SetError( err, FSARCH_ERROR_FILE_NOT_FOUND );
	}
	
	virtual int RmDir( FSPath& path, int* err, FSCInfo* info ) override
	{
		return FS::SetError( err, FSARCH_ERROR_FILE_NOT_FOUND );
	}
	
	virtual int SetFileTime( FSPath& path, FSTime cTime, FSTime aTime, FSTime mTime, int* err, FSCInfo* info ) override
	{
		return FS::SetError( err, FSARCH_ERROR_FILE_NOT_FOUND );
	}
	
	virtual int ReadDir( FSList* list, FSPath& path, int* err, FSCInfo* info ) override;
	virtual int Stat( FSPath& path, FSStat* st, int* err, FSCInfo* info ) override;
	virtual int StatSetAttr( FSPath& path, const FSStat* st, int* err, FSCInfo* info ) override
	{
		return FS::SetError( err, FSARCH_ERROR_FILE_NOT_FOUND );
	}
	
	virtual int FStat( int fd, FSStat* st, int* err, FSCInfo* info ) override
	{
		return FS::SetError( err, FSARCH_ERROR_FILE_NOT_FOUND );
	}
	
	virtual int Symlink( FSPath& path, FSString& str, int* err, FSCInfo* info ) override {  return -1;  }
	virtual int StatVfs( FSPath& path, FSStatVfs* st, int* err, FSCInfo* info ) override;
	
	virtual int64_t GetFileSystemFreeSpace( FSPath& path, int* err ) override { return 0; }
	
	virtual FSString Uri( FSPath& path ) override
	{
		std::string ret(std::string("Archive://") + path.GetUtf8('/'));
		return FSString(ret.c_str());
	}
};


FSArchNode::FSArchNode()
	: parentDir( nullptr )
{
	fsStat.mode |= S_IFDIR;

#ifdef _WIN32
	fsStat.m_CreationTime = FSTime( FSTime::TIME_CURRENT );
	fsStat.m_LastAccessTime = FSTime( FSTime::TIME_CURRENT );
	fsStat.m_LastWriteTime = FSTime( FSTime::TIME_CURRENT );
	fsStat.m_ChangeTime = FSTime( FSTime::TIME_CURRENT );
#else
	fsStat.m_CreationTime = time( 0 );
	fsStat.m_LastAccessTime = time( 0 );
	fsStat.m_LastWriteTime = time( 0 );
	fsStat.m_ChangeTime = time( 0 );
#endif
}

FSArchNode* FSArchNode::findByFsPath( FSPath& fsPath, int fsPathLevel )
{
	FSString* curName = fsPath.GetItem( fsPathLevel );

	if ( parentDir == nullptr
		&& fsPathLevel == fsPath.Count() - 1
		&& ( curName == nullptr || curName->GetUnicode() == 0 || curName->GetUnicode()[0] == 0 ) )
	{
		// request to find root node, and we are root
		return this;
	}

	if ( name.Cmp( *curName ) == 0 )
	{
		if ( fsPath.Count() <= fsPathLevel )
		{
			// exact match
			return this;
		}

		FSString* childName = fsPath.GetItem( fsPathLevel + 1 );
		FSArchNode* n = findByName( *childName, false );
		if ( n == nullptr )
		{
			return nullptr;
		}

		if ( fsPath.Count() <= fsPathLevel + 2 )
		{
			// no further recursion
			return n;
		}
		
		if ( n->IsDir() )
		{
			// recurse into subdir
			return n->findByFsPath( fsPath, fsPathLevel + 1 );
		}
	}

	return nullptr;
}

FSArchNode* FSArchNode::findByName( const FSString& name, bool isRecursive )
{
	//printf("FSArchNodeDir::findByName name=%s\n",name->GetUtf8());
	// first, try all nodes in current
	for ( FSArchNode& n : content )
	{
		//printf("FSArchNodeDir::findByName *it.name=%s\n", (*it).name.GetUtf8());
		if ( n.name.Cmp( (FSString&) name ) == 0 )
		{
			return &n;
		}
	}

	// only if not found in current dir, try inside subdirs
	if ( isRecursive )
	{
		for ( FSArchNode& n : content )
		{
			if ( n.IsDir() )
			{
				FSArchNode* pn = n.findByName( name, isRecursive );
				if ( pn )
				{
					return pn;
				}
			}
		}
	}

	return nullptr;
}

bool FSArchNode::Remove( const FSString& name, bool searchRecursive )
{
	// first, try all nodes in current dir
	for ( auto it = content.begin(); it != content.end(); ++it )
	{
		if ( ( *it ).name.Cmp( (FSString&) name ) == 0 )
		{
			content.erase( it );
			return true;
		}
	}

	// only if not found in current dir, try inside subdirs
	if ( searchRecursive )
	{
		for ( FSArchNode& n : content )
		{
			if ( n.IsDir() )
			{
				const bool ret = n.Remove( name, searchRecursive );
				if ( ret )
				{
					return true;
				}
			}
		}
	}

	return false;
}

FSArchNode* FSArchNode::Add( const FSArchNode& fsArchNode )
{
	if ( findByName( fsArchNode.name ) )
	{
		return nullptr;
	}

	content.push_back( fsArchNode );
	
	FSArchNode* Added = &content.back();
	Added->parentDir = this;
	return Added;
}

//
// FSArch
//

int FSArch::ReadDir( FSList* list, FSPath& path, int* err, FSCInfo* info )
{
	list->Clear();

	if ( path.Count() == 1 )
	{
		// Adding the ".." node to root dir
		clPtr<FSNode> pNode = new FSNode();
		pNode->name = std::string( ".." );
		pNode->st.mode = S_IFDIR;
		list->Append( pNode );
	}

	FSArchNode* n = rootDir.findByFsPath( path );
	if ( !n || !n->IsDir() )
	{
		return FS::SetError( err, FSARCH_ERROR_FILE_NOT_FOUND );
	}

	for ( FSArchNode& tn : n->content )
	{
		clPtr<FSNode> pNode = new FSNode();
		pNode->name = ( std::string( "" ) + tn.name.GetUtf8() ).c_str();
		pNode->st = tn.fsStat;

		list->Append( pNode );
	}

	return 0;
}

int FSArch::Stat( FSPath& path, FSStat* st, int* err, FSCInfo* info )
{
	//printf("FSArch::Stat ");
	FSArchNode* n = rootDir.findByFsPath( path );
	if ( !n )
	{
		return FS::SetError( err, FSARCH_ERROR_FILE_NOT_FOUND );
	}

	*st = n->fsStat;
	return FS::SetError( err, 0 );
}

int FSArch::StatVfs( FSPath& path, FSStatVfs* st, int* err, FSCInfo* info )
{
	//printf("FSArch::StatVfs ");
	*st = FSStatVfs();
	return 0;
}

//
// clArchPlugin
//

const char* clArchPlugin::GetPluginId() const
{
	return g_ArchPluginId;
}

clPtr<FS> clArchPlugin::OpenFileVFS( clPtr<FS> Fs, FSPath& Path, const std::string& FileExtLower ) const
{
	FSArchNode RootDir;
	
	FSStat Stat;

	Stat.mode |= S_IFDIR;
	Stat.size = 0;
	FSArchNode* Dir1 = RootDir.Add( FSArchNode( "test dir 1", Stat ) );
	
	Stat.mode |= S_IFREG;
	Stat.size = 1;
	Dir1->Add( FSArchNode( "test file 1", Stat ) );
	
	Stat.mode |= S_IFREG;
	Stat.size = 2;
	FSArchNode File2( "test file 2", Stat );
	RootDir.Add( File2 );

	return new FSArch( RootDir );
}
