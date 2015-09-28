/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * wcm@linderdaum.com
 */

#ifdef LIBARCHIVE_EXIST

#include "plugin-archive.h"
#include "vfs.h"
#include "panel.h"
#include "string-util.h"

#include <list>
#include <algorithm>

#include <archive.h>
#include <archive_entry.h>

//#define dbg_printf printf

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
	int64_t m_EntryOffset; // position of entry inside archive
	
	struct FSArchNode* parentDir;
	std::list<FSArchNode> content; // content of a dir node
	
	FSArchNode()
		: m_EntryOffset( 0 ), parentDir( nullptr ) { fsStat.mode |= S_IFDIR; }

	FSArchNode( const char* Name, const FSStat& Stat )
		: name( Name ), fsStat( Stat ),  m_EntryOffset( 0 ), parentDir( nullptr ) {}
	
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
	FSString FileName; // original file name
	
public:
	FSArch( const FSArchNode& RootDir, const FSString& FileName ) : FS( ARCH ), rootDir( RootDir ), FileName( FileName ) {}
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
	
	virtual int64_t GetFileSystemFreeSpace( FSPath& path, int* err ) override { return rootDir.m_EntryOffset; }
	
	virtual FSString Uri( FSPath& path ) override
	{
		std::string ret( FileName.GetUtf8() + std::string(":") + path.GetUtf8('/'));
		return FSString(ret.c_str());
	}
};


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
	dbg_printf( "FSArchNodeDir::findByName name=%s\n", name.GetUtf8() );
	
	// first, try all nodes in current
	for ( FSArchNode& n : content )
	{
		dbg_printf( "FSArchNodeDir::findByName n.name=%s\n", n.name.GetUtf8() );

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

	if ( path.Count() == 1 && !g_WcmConfig.panelShowDotsInRoot)
	{
		// Adding the ".." node to the root dir
		clPtr<FSNode> pNode = new FSNode();
		pNode->name = std::string( ".." );
		pNode->st = rootDir.fsStat;
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
	dbg_printf( "FSArch::Stat " );

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
	dbg_printf( "FSArch::StatVfs " );

	FSStatVfs StatVfs;
	StatVfs.avail = 0;
	StatVfs.size = rootDir.m_EntryOffset;

	*st = StatVfs;
	return 0;
}

//
// clArchPlugin
//

const char* clArchPlugin::GetPluginId() const
{
	return g_ArchPluginId;
}

void ArchClose( struct archive* Arch )
{
	if ( Arch != nullptr )
	{
		int Res = archive_read_free( Arch );
		if ( Res != ARCHIVE_OK )
		{
			printf( "Couldn't close archive reader: %s\n", archive_error_string( Arch ) );
		}
	}
}

struct archive* ArchOpen( const char* FileName )
{
	struct archive* Arch = archive_read_new();
	if ( Arch != nullptr )
	{
		int Res = archive_read_support_filter_all( Arch );
		if ( Res == ARCHIVE_OK )
		{
			Res = archive_read_support_format_all( Arch );
			if ( Res == ARCHIVE_OK )
			{
				Res = archive_read_open_filename( Arch, FileName, 10240 );
				if ( Res == ARCHIVE_OK )
				{
					return Arch;
				}
				
				dbg_printf( "Couldn't open input archive: %s\n", archive_error_string( Arch ) );
			}
			else
			{
				dbg_printf( "Couldn't enable read formats\n" );
			}
		}
		else
		{
			dbg_printf( "Couldn't enable decompression\n" );
		}
	}
	else
	{
		dbg_printf( "Couldn't create archive reader\n" );
	}
	
	ArchClose( Arch );
	return nullptr;
}

FSArchNode* ArchGetParentDir( FSArchNode* CurrDir, FSPath& ItemPath, const FSStat& ItemStat )
{
	for ( int i = 0; i < ItemPath.Count() - 1; i++ )
	{
		FSString* Name = ItemPath.GetItem( i );
		if ( i == 0 && ( Name->IsDot() || Name->IsEmpty() ) )
		{
			// skip root dir
			continue;
		}
		
		FSArchNode* Dir = CurrDir->findByName( *Name, false );
		if ( Dir == nullptr )
		{
			FSStat Stat = ItemStat;
			Stat.mode = S_IFDIR;
			Stat.size = 0;
			Dir = CurrDir->Add( FSArchNode( Name->GetUtf8(), Stat ) );
		}
		
		CurrDir = Dir;
	}
	
	return CurrDir;
}

clPtr<FS> clArchPlugin::OpenFileVFS( clPtr<FS> Fs, FSPath& Path, const FSNode& Node, const std::string& FileExtLower ) const
{
	struct archive* Arch = ArchOpen( Fs->Uri( Path ).GetUtf8() );
	if ( Arch == nullptr )
	{
		return nullptr;
	}

	FSArchNode RootDir;
	RootDir.fsStat = Node.st;
	RootDir.fsStat.mode = S_IFDIR;

	FSPath NodePath;
	struct archive_entry* entry = archive_entry_new2( Arch );
	
	int Res;
	while ( ( Res = archive_read_next_header2( Arch, entry ) ) == ARCHIVE_OK )
	{
		NodePath.Set( CS_UTF8, archive_entry_pathname( entry ) );

		FSString* ItemName = NodePath.GetItem( NodePath.Count() - 1 );
		if ( NodePath.Count() == 1 && ( ItemName->IsDot() || ItemName->IsEmpty() ) )
		{
			// skip root dir
			continue;
		}

		const mode_t Mode = archive_entry_mode( entry );
		const int64_t Size = archive_entry_size( entry );
		RootDir.m_EntryOffset += Size;
		
		FSStat ItemStat;
		ItemStat.mode = S_ISREG( Mode ) ? Mode : S_IFDIR;
		ItemStat.size = Size;
		ItemStat.m_CreationTime = archive_entry_ctime( entry );
		ItemStat.m_LastAccessTime = archive_entry_atime( entry );
		ItemStat.m_LastWriteTime = archive_entry_mtime( entry );

		FSArchNode* Dir = ArchGetParentDir( &RootDir, NodePath, ItemStat );
		FSArchNode* Item = Dir->Add( FSArchNode( ItemName->GetUtf8(), ItemStat ) );
		Item->m_EntryOffset = archive_read_header_position( Arch );
	}
	
	if ( Res != ARCHIVE_EOF )
	{
		dbg_printf( "Couldn't read archive entry: %s\n", archive_error_string( Arch ) );
	}
	
	archive_entry_free(entry);
	ArchClose( Arch );

	return new FSArch( RootDir, Node.GetUtf8Name() );
}

#endif //LIBARCHIVE_EXIST
