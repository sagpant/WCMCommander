/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * wcm@linderdaum.com
 */

#ifdef _WIN32
#	define _CRT_SECURE_NO_WARNINGS
#endif

#include "file-util.h"
#include "ncwin.h"
#include "string-util.h"
#include "vfs.h"
#include "vfs-uri.h"
#include "fileopers.h"

#include <inttypes.h>  // for PRIX64 macro (and INT64_C macro from stdint.h)
#include <unordered_map>


/// Next Temp directory ID, incremented each time a new Temp directory is created
static int g_NextTempDirId = 1;

/// Temp directories ID to URI global list
static std::unordered_map<int, std::vector<unicode_t>> g_TempDirUriList;


bool DeleteDirRecursively( FS* fs, FSPath path );

bool DeleteListRecursively( FS* fs, FSPath path, FSList& list )
{
	const int cnt = path.Count();
	int ret_err;

	for ( FSNode* node = list.First(); node; node = node->next )
	{
		if ( node->extType )
		{
			continue;
		}

		path.SetItemStr( cnt, node->Name() );

		if ( node->IsDir() && !node->st.IsLnk() )
		{
			if ( !DeleteDirRecursively( fs, path ) )
			{
				return false;
			}
		}
		else if ( fs->Delete( path, &ret_err, nullptr ) != 0 )
		{
			return false;
		}
	}

	return true;
}

/// Deletes dir recursively
bool DeleteDirRecursively( FS* fs, FSPath path )
{
	FSList list;
	int ret_err;
	int ret = fs->ReadDir( &list, path, &ret_err, nullptr );
	if ( ret == 0 )
	{
		DeleteListRecursively( fs, path, list );
		return ( fs->RmDir( path, &ret_err, nullptr ) == 0 );
	}

	return false;
}


#ifdef _WIN32

std::vector<unicode_t> GetHomeUriWin()
{
	wchar_t homeDrive[0x100];
	wchar_t homePath[0x100];
	int l1 = GetEnvironmentVariableW( L"HOMEDRIVE", homeDrive, 0x100 );
	int l2 = GetEnvironmentVariableW( L"HOMEPATH", homePath, 0x100 );

	if ( l1 > 0 && l1 < 0x100 && l2 > 0 && l2 < 0x100 )
	{
		return carray_cat<unicode_t>( Utf16ToUnicode( homeDrive ).data(), Utf16ToUnicode( homePath ).data() );
	}

	return std::vector<unicode_t>();
}

std::vector<unicode_t> GetTempUriWin()
{
	wchar_t TempPath[0x100];
	int l1 = GetEnvironmentVariableW( L"TMP", TempPath, 0x100 );

	if ( l1 > 0 && l1 < 0x100 )
	{
		return Utf16ToUnicode( TempPath );
	}

	return std::vector<unicode_t>();
}

#endif // _WIN32


bool GetSysTempDir( clPtr<FS>* fs, FSPath* path )
{
#ifdef _WIN32
	std::vector<unicode_t> TempUri = GetTempUriWin();

	if ( TempUri.data() )
	{
		*fs = ParzeURI( TempUri.data(), *path, std::vector<clPtr<FS>>() );
		
		return !fs->IsNull();
	}

	return false;

#else
	const sys_char_t* Temp = (sys_char_t*) getenv( "TMPDIR" );
	if ( !Temp )
	{
		/*
		 * The Filesystem Hierarchy Standard version 3.0 says:
		 * /tmp : Temporary files
		 * The /tmp directory must be made available for programs that require temporary files.
		 */
		Temp = (sys_char_t*) "/tmp";
	}


	path->Set( sys_charset_id, Temp );
	*fs = new FSSys();
	return true;
#endif
}

int CreateWcmTempDir( clPtr<FS>* fs, FSPath* path )
{
	if ( !GetSysTempDir( fs, path ) )
	{
		return 0;
	}

	const int Id = g_NextTempDirId++;

	time_t TimeInSeconds = time( 0 ); // get time now

	// convert current time from int64 to hex form
	// got from here: http://stackoverflow.com/questions/7240391/decimal-to-hex-for-int64-in-c
	char Buf[128];
	sprintf( Buf, "WCM%" PRIx64 ".%d", (int64_t)TimeInSeconds, Id );

	path->Push( CS_UTF8, Buf );
	
	int ret_err;
	if ( fs->Ptr()->MkDir( *path, 0777, &ret_err, nullptr ) == 0 )
	{
		// store created temp dir for later cleanup
		g_TempDirUriList[Id] = new_unicode_str( (*fs)->Uri( *path ).GetUnicode() );
	}

	return Id;
}

void RemoveWcmTempDir( const int Id )
{
	auto iter = g_TempDirUriList.find( Id );

	if ( iter != g_TempDirUriList.end() )
	{
		std::vector<unicode_t> TempUri = iter->second;

		if ( TempUri.data() )
		{
			FSPath path;
			clPtr<FS> fs = ParzeURI( TempUri.data(), path, std::vector<clPtr<FS>>() );
			
			DeleteDirRecursively( fs.Ptr(), path );
		}
	}
}

void RemoveAllWcmTempDirs()
{
	for ( auto &iter : g_TempDirUriList )
	{
		std::vector<unicode_t> TempUri = iter.second;

		if ( TempUri.data() )
		{
			FSPath path;
			clPtr<FS> fs = ParzeURI( TempUri.data(), path, std::vector<clPtr<FS>>() );

			DeleteDirRecursively( fs.Ptr(), path );
		}
	}
}

void OpenHomeDir( PanelWin* p )
{
#ifdef _WIN32
	std::vector<unicode_t> homeUri = GetHomeUriWin();

	if ( homeUri.data() )
	{
		const std::vector<clPtr<FS>> checkFS =
		{
			p->GetFSPtr(),
			g_MainWin->GetOtherPanel( p )->GetFSPtr()
		};

		FSPath path;
		clPtr<FS> fs = ParzeURI( homeUri.data(), path, checkFS );

		if ( fs.IsNull() )
		{
			char buf[4096];
			FSString name = homeUri.data();
			Lsnprintf( buf, sizeof( buf ), "bad home path: %s\n", name.GetUtf8() );
			NCMessageBox( g_MainWin, "Home", buf, true );
		}
		else
		{
			p->LoadPath( fs, path, 0, 0, PanelWin::SET );
		}
	}

#else
	const sys_char_t* home = (sys_char_t*) getenv( "HOME" );
	if ( !home )
	{
		return;
	}

	FSPath path( sys_charset_id, home );
	p->LoadPath( new FSSys(), path, 0, 0, PanelWin::SET );
#endif
}


//////////////////////////////// Load file data


class LoadFileDataThreadData : public OperData
{
public:
	clPtr<FS>& m_SrcFs;
	FSPath& m_SrcPath;
	clPtr<FS>& m_DstFs;
	FSPath& m_DstPath;
	FSString m_ErrorString;

	bool m_Success;
	
	LoadFileDataThreadData( NCDialogParent* p, clPtr<FS>& srcFs, FSPath& srcPath, clPtr<FS>& dstFs, FSPath& dstPath )
		: OperData( p )
		, m_SrcFs( srcFs )
		, m_SrcPath( srcPath )
		, m_DstFs( dstFs )
		, m_DstPath( dstPath )
		, m_ErrorString( "" )
		, m_Success( false )
	{
	}
};


class LoadFileDataThread : public OperFileThread
{
public:
	LoadFileDataThread( const char* opName, NCDialogParent* par, OperThreadNode* n )
		: OperFileThread( opName, par, n ) {}

	virtual void Run() override;
};

void LoadFileDataThread::Run()
{
	MutexLock lock( Node().GetMutex() ); //!!!

	if ( Node().NBStopped() )
	{
		return;
	}

	LoadFileDataThreadData* Data = ((LoadFileDataThreadData*) Node().Data());
	clPtr<FS>& srcFs = Data->m_SrcFs;
	FSPath& srcPath = Data->m_SrcPath;
	clPtr<FS>& dstFs = Data->m_DstFs;
	FSPath& dstPath = Data->m_DstPath;
	lock.Unlock();

	int ret_error;
	int in = -1;
	int out = -1;

	try
	{
		in = srcFs->OpenRead( srcPath, FS::SHARE_READ, &ret_error, Info() );
		if ( in < 0 )
		{
			throw_msg( "%s", srcFs->StrError( ret_error ).GetUtf8() );
		}
		
		out = dstFs->OpenCreate( dstPath, true, 0666, 0, &ret_error, Info() );
		if ( out < 0 )
		{
			throw_msg( "%s", dstFs->StrError( ret_error ).GetUtf8() );
		}

		char buf[16 * 1024];

		while ( true )
		{
			lock.Lock();
			if ( Node().NBStopped() )
			{
				// stopped by user pressing Cancel button
				lock.Unlock();
				break;
			}

			lock.Unlock();

			const int n = srcFs->Read( in, buf, sizeof( buf ), &ret_error, Info() );
			if ( n < 0 )
			{
				throw_msg( "Can't load file:\n%s", srcFs->StrError( ret_error ).GetUtf8() );
			}

			if ( n == 0 )
			{
				// we reached end of file
				lock.Lock();
				Data->m_Success = true;
				lock.Unlock();
				break;
			}

			const int b = dstFs->Write( out, buf, n, &ret_error, Info() );
			if ( b < 0 )
			{
				throw_msg( "Can't write file:\n%s", dstFs->StrError( ret_error ).GetUtf8() );
			}
			
			if ( b != n )
			{
				throw_msg( "Cannot save data, maybe disk is full" );
			}
		}

		srcFs->Close( in, 0, Info() );
		in = -1;

		dstFs->Close( out, 0, Info() );
		out = -1;
	}
	catch ( ... )
	{
		if ( in >= 0 )
		{
			srcFs->Close( in, 0, Info() );
		}
		if ( out >= 0 )
		{
			dstFs->Close( out, 0, Info() );
		}

		throw;
	}
}


void LoadFileDataThreadFunc( OperThreadNode* node )
{
	try
	{
		MutexLock lock( node->GetMutex() );
		if ( !node->Data() )
		{
			return;
		}

		LoadFileDataThreadData* Data = ((LoadFileDataThreadData*) node->Data());
		LoadFileDataThread thread( "Load file", Data->Parent(), node );
		lock.Unlock();

		try
		{
			thread.Run();
		}
		catch ( cexception* ex )
		{
			lock.Lock(); //!!!

			if ( !node->NBStopped() ) //must always be checked, otherwise data maybe invalid
			{
				Data->m_ErrorString = ex->message();
			}

			lock.Unlock();
			ex->destroy();
		}
	}
	catch ( cexception* ex )
	{
		fprintf( stderr, "ERR!!! Exception in LoadFileDataThreadFunc - '%s'\n", ex->message() );
		ex->destroy();
	}
	catch ( ... )
	{
		fprintf( stderr, "ERR!!! Unhandled exception in LoadFileDataThreadFunc\n" );
	}
}


class LoadFileDataThreadDlg : public NCDialog
{
public:
	LoadFileDataThreadData m_Data;

	LoadFileDataThreadDlg( NCDialogParent* parent, clPtr<FS>& srcFs, FSPath& srcPath, clPtr<FS>& dstFs, FSPath& dstPath )
		: NCDialog( ::createDialogAsChild, 0, parent, utf8_to_unicode( "Loading file" ).data(), bListCancel )
		, m_Data( parent, srcFs, srcPath, dstFs, dstPath )
	{
	}
	
	virtual void OperThreadStopped() override;

	virtual void OnCancel() override
	{
		SetStopFlag();
	}
};

void LoadFileDataThreadDlg::OperThreadStopped()
{
	if ( !m_Data.m_ErrorString.IsEmpty() )
	{
		NCMessageBox( (NCDialogParent*) Parent(), "Failed to load file data", m_Data.m_ErrorString.GetUtf8(), true );
		EndModal( 0 );
		return;
	}

	EndModal( CMD_OK );
}


int LoadToTempFile( NCDialogParent* parent, clPtr<FS>* fs, FSPath* path )
{
	clPtr<FS> TempFs;
	FSPath TempPath;
	const int TempId = CreateWcmTempDir( &TempFs, &TempPath );
	if ( !TempId )
	{
		return 0;
	}

	// append file name to the created temp dir
	FSPath DstPath = TempPath;
	DstPath.Push( CS_UTF8, path->GetItem( path->Count() - 1 )->GetUtf8() );

	LoadFileDataThreadDlg dlg( parent, *fs, *path, TempFs, DstPath );
	dlg.RunNewThread( "Load file", LoadFileDataThreadFunc, &dlg.m_Data );
	dlg.DoModal();
	
	if ( !dlg.m_Data.m_Success )
	{
		// cleanup created temp dir
		RemoveWcmTempDir( TempId );
		return 0;
	}
	
	*fs = TempFs;
	*path = DstPath;
	return TempId;
}
