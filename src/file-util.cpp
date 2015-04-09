/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * walcommander@linderdaum.com
 */

#include "file-util.h"
#include "string-util.h"
#include "vfs.h"
#include "vfs-uri.h"
#include "fileopers.h"


#ifdef _WIN32

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
		const std::vector<clPtr<FS>> checkFS;
		*fs = ParzeURI( TempUri.data(), *path, checkFS );
		
		return !fs->IsNull();
	}

	return false;

#else
	const sys_char_t* Temp = (sys_char_t*) getenv( "TMPDIR" );
	if ( !Temp )
	{
		return;
	}

	path->Set( sys_charset_id, Temp );
	*fs = new FSSys();
	return true;
#endif
}

bool CreateWcmTempDir( clPtr<FS>* fs, FSPath* path )
{
	if ( !GetSysTempDir( fs, path ) )
	{
		return false;
	}

	path->Push( CS_UTF8, "WCM.tmp" );
	
	int ret_err;
	return ( fs->Ptr()->MkDir( *path, 0777, &ret_err, nullptr ) == 0 );
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
		, m_Success( false )
		, m_SrcFs( srcFs )
		, m_SrcPath( srcPath )
		, m_DstFs( dstFs )
		, m_DstPath( dstPath )
		, m_ErrorString( "" )
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


bool LoadToTempFile( NCDialogParent* parent, clPtr<FS>* fs, FSPath* path )
{
	clPtr<FS> dstFs;
	FSPath dstPath;
	if ( !CreateWcmTempDir( &dstFs, &dstPath ) )
	{
		return false;
	}

	// append file name to the created temp dir
	dstPath.Push( CS_UTF8, path->GetItem( path->Count() - 1 )->GetUtf8() );

	LoadFileDataThreadDlg dlg( parent, *fs, *path, dstFs, dstPath );
	dlg.RunNewThread( "Load file", LoadFileDataThreadFunc, &dlg.m_Data ); //throw
	dlg.DoModal();
	
	if ( dlg.m_Data.m_Success )
	{
		*fs = dstFs;
		*path = dstPath;
	}

	return dlg.m_Data.m_Success;
}
