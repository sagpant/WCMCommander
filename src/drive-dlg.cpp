/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * walcommander@linderdaum.com
 */

#include "drive-dlg.h"
#include "ncwin.h"
#include "globals.h"
#include "string-util.h"
#include "ltext.h"
#include "vfs-ftp.h"
#include "vfs-tmp.h"
#include "ftplogon.h"

#if defined(LIBSSH_EXIST) || defined(LIBSSH2_EXIST)
#  include "sftpdlg.h"
#endif

#ifndef _WIN32
#  include "ux_util.h"
#else
#  include "w32util.h"
#endif


static int uiDriveDlg = GetUiID( "drive-dlg" );

#ifdef _WIN32

std::string GetVolumeName( int i )
{
	char VolumeName[1024] = { 0 };

	char RootPath[0x100];
	Lsnprintf( RootPath, sizeof( RootPath ), "%c:\\", i + 'A' );
	GetVolumeInformation( RootPath, VolumeName, sizeof( VolumeName ), nullptr, nullptr, nullptr, nullptr, 0 );

	return std::string( VolumeName );
}

std::string GetDriveTypeString( unsigned int Type )
{
	switch ( Type )
	{
		case DRIVE_REMOVABLE:
			return "removable";

		case DRIVE_FIXED:
			return "fixed";

		case DRIVE_REMOTE:
			return "remote";

		case DRIVE_CDROM:
			return "CDROM";

		case DRIVE_RAMDISK:
			return "RAM disk";
	}

	return std::string();
}

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

#endif // _WIN32


#ifdef __APPLE__

#define VOLUMES_DIR	"/Volumes"

void GetVolumesOSX( ccollect<MntListNode>* MntList )
{
	FSSys fs;
	FSList list;
	FSString path( VOLUMES_DIR );
	FSPath fspath( path );
	int err;
	
	if ( fs.ReadDir( &list, fspath, &err, nullptr ) == 0 )
	{
		for ( FSNode* node = list.First(); node; node = node->next )
		{
			if ( node->IsDir() && !node->IsHidden() && !node->IsLnk() )
			{
				std::vector<char> Uri = carray_cat( VOLUMES_DIR, "/", node->name.GetUtf8() );
				
				MntListNode n;
				n.path = Uri.data();
				MntList->append( n );
			}
		}
	}
	else
	{
		fprintf( stderr, "error:%s\n", fs.StrError( err ).GetUtf8() );
	}
}

#endif // __APPLE__


/// returns 'true' if the dialog should be restarted
bool SelectDriveInternal( PanelWin* p, PanelWin* OtherPanel )
{
#ifndef _WIN32
	ccollect<MntListNode> mntList;

#ifdef __APPLE__
	GetVolumesOSX( &mntList );
#else
	UxMntList( &mntList );
#endif

#endif // !_WIN32

	clMenuData mData;

	std::vector<unicode_t> OtherPanelPath = new_unicode_str( OtherPanel->UriOfDir().GetUnicode() );

#ifdef _WIN32
	const size_t MaxLength = 20;
#else
	const size_t MaxLength = 50;
#endif

	OtherPanelPath = TruncateToLength( OtherPanelPath, MaxLength, true );

	mData.Add( OtherPanelPath.data(), nullptr, nullptr, ID_DEV_OTHER_PANEL );
	mData.AddSplitter();

#ifdef _WIN32
	std::vector<unicode_t> homeUri = GetHomeUriWin();

	if ( homeUri.data() )
	{
		mData.Add( _LT( "Home" ), nullptr, nullptr, ID_DEV_HOME );
	}

	DWORD drv = GetLogicalDrives();

	for ( int i = 0, mask = 1; i < 'z' - 'a' + 1; i++, mask <<= 1 )
	{
		if ( drv & mask )
		{
			char buf[0x100];
			Lsnprintf( buf, sizeof( buf ), "%c:", i + 'A' );
			UINT driveType = GetDriveType( buf );

			bool ShouldReadVolumeName = ( driveType == DRIVE_FIXED || driveType == DRIVE_RAMDISK );

			std::string DriveTypeStr = GetDriveTypeString( driveType );
			std::string VolumeName = ShouldReadVolumeName ? GetVolumeName( i ) : std::string();

			mData.Add( buf, DriveTypeStr.c_str(), VolumeName.c_str(), ID_DEV_MS0 + i );
		}
	}

	mData.AddSplitter();
	mData.Add( "1. NETWORK", nullptr, nullptr, ID_DEV_SMB );
	mData.Add( "2. FTP", nullptr, nullptr, ID_DEV_FTP );
#else
	mData.Add( "1. /", nullptr, nullptr,  ID_DEV_ROOT );
	mData.Add( _LT( "2. Home" ), nullptr, nullptr, ID_DEV_HOME );
	mData.Add( "3. FTP", nullptr, nullptr, ID_DEV_FTP );

#ifdef LIBSMBCLIENT_EXIST
	mData.Add( "4. Smb network", nullptr, nullptr, ID_DEV_SMB );
	mData.Add( "5. Smb server", nullptr, nullptr, ID_DEV_SMB_SERVER );
#else

#endif // LIBSMBCLIENT_EXIST

#endif // _WIN32

#if defined(LIBSSH_EXIST) || defined(LIBSSH2_EXIST)
	mData.Add( "6. SFTP", nullptr, nullptr, ID_DEV_SFTP );
#endif
	mData.Add( "7. temporary", nullptr, nullptr, ID_DEV_TMP );

#ifndef _WIN32  //unix mounts
	//ID_MNT_UX0
	{
		if ( mntList.count() > 0 )
		{
			mData.AddSplitter();
		}

		for ( int i = 0; i < 9 && i < mntList.count(); i++ )
		{
			std::string Uri = mntList[i].path;

#ifdef __APPLE__
			// truncate /Volumes/ dir prefix
			Uri = Uri.substr( strlen( VOLUMES_DIR ) + 1, Uri.size() );
#endif
			
			std::vector<unicode_t> un = sys_to_unicode_array( Uri.data() );
			static int maxNLen = 20;
			int nLen = unicode_strlen( un.data() );

			if ( nLen > maxNLen )
			{
				int n2 = maxNLen / 2;
				un[n2] = 0;
				un[n2 - 1] = '.';
				un[n2 - 2] = '.';
				un[n2 - 3] = '.';
				un = carray_cat<unicode_t>( un.data(), un.data() + nLen - n2 );
			}

			std::vector<unicode_t> ut = sys_to_unicode_array( mntList[i].type.data() );

			static int maxTLen = 10;

			if ( unicode_strlen( ut.data() ) > maxTLen )
			{
				ut[maxTLen] = 0;
				ut[maxTLen - 1] = '.';
				ut[maxTLen - 2] = '.';
				ut[maxTLen - 3] = '.';
			}

			char buf[64];
			snprintf( buf, sizeof( buf ), "%i ", i + 1 );

			mData.Add( carray_cat<unicode_t>( utf8_to_unicode( buf ).data(), un.data() ).data(), ut.data(), nullptr, ID_MNT_UX0 + i );
		}
	}
#endif

	const int res = RunDldMenu( uiDriveDlg, p, "Drive", &mData );
	g_MainWin->SetCommandLineFocus();

	if ( res == ID_RESTART_DIALOG )
	{
		// restart this dialog to reread the drives list (Ctrl+R)
		return true;
	}

	if ( res == ID_DEV_OTHER_PANEL )
	{
		clPtr<FS> fs = OtherPanel->GetFSPtr();
		p->LoadPath( fs, OtherPanel->GetPath(), 0, 0, PanelWin::SET );
		return false;
	}

#ifdef _WIN32

	if ( res >= ID_DEV_MS0 && res < ID_DEV_MS0 + ( 'z' - 'a' + 1 ) )
	{
		int drive = res - ID_DEV_MS0;
		FSPath path( CS_UTF8, "/" );
		clPtr<FS> fs = p->GetFSPtr();

		if ( !fs.IsNull() && fs->Type() == FS::SYSTEM && ( ( FSSys* )fs.Ptr() )->Drive() == drive )
		{
			path = p->GetPath();
		}
		else
		{
			fs = OtherPanel->GetFSPtr();

			if ( !fs.IsNull() && fs->Type() == FS::SYSTEM && ( ( FSSys* )fs.Ptr() )->Drive() == drive )
			{
				path = OtherPanel->GetPath();
			}
			else
			{
				fs = 0;
			}
		}

		if ( fs.IsNull() )
		{
			fs = new FSSys( drive );
		}

		if ( !path.IsAbsolute() )
		{
			path.Set( CS_UTF8, "/" );
		}

		p->LoadPath( fs, path, 0, 0, PanelWin::SET );
		return false;
	}

#else

	if ( res >= ID_MNT_UX0 && res < ID_MNT_UX0 + 100 )
	{
		const int n = res - ID_MNT_UX0;

		if ( n < 0 || n >= mntList.count() )
		{
			return false;
		}

		clPtr<FS> fs = new FSSys();
		FSPath path( sys_charset_id, mntList[n].path.data() );
		p->LoadPath( fs, path, 0, 0, PanelWin::SET );
		return false;
	}

#endif

	switch ( res )
	{
#ifndef _WIN32

		case ID_DEV_ROOT:
		{
			FSPath path( CS_UTF8, "/" );
			p->LoadPath( new FSSys(), path, 0, 0, PanelWin::SET );
		}
		break;
#endif

		case ID_DEV_HOME:
		{
			OpenHomeDir( p, OtherPanel );
		}
		break;

#ifdef _WIN32

		case ID_DEV_SMB:
		{
			FSPath path( CS_UTF8, "/" );
			clPtr<FS> fs = new FSWin32Net( 0 );

			if ( !fs.IsNull() )
			{
				p->LoadPath( fs, path, 0, 0, PanelWin::SET );
			}
		}
		break;

#else

#ifdef LIBSMBCLIENT_EXIST

		case ID_DEV_SMB:
		{
			FSPath path( CS_UTF8, "/" );
			clPtr<FS> fs = new FSSmb() ;

			if ( !fs.IsNull() )
			{
				p->LoadPath( fs, path, 0, 0, PanelWin::SET );
			}
		}
		break;

		case ID_DEV_SMB_SERVER:
		{
			static FSSmbParam lastParams;
			FSSmbParam params = lastParams;

			if ( !GetSmbLogon( g_MainWin, params, true ) )
			{
				return false;
			}

			params.isSet = true;
			clPtr<FS> fs = new FSSmb( &params ) ;
			FSPath path( CS_UTF8, "/" );

			if ( !fs.IsNull() )
			{
				lastParams = params;
				p->LoadPath( fs, path, 0, 0, PanelWin::SET );
			}
		}
		break;

#endif

#endif

		case ID_DEV_FTP:
		{
			static FSFtpParam lastParams;
			FSFtpParam params = lastParams;

			if ( !GetFtpLogon( g_MainWin, params ) )
			{
				return false;
			}

			clPtr<FS> fs = new FSFtp( &params ) ;

			FSPath path( CS_UTF8, "/" );

			if ( !fs.IsNull() )
			{
				lastParams = params;
				p->LoadPath( fs, path, 0, 0, PanelWin::SET );
			}
		}
		break;

#if defined(LIBSSH_EXIST) || defined(LIBSSH2_EXIST)

		case ID_DEV_SFTP:
		{
			static FSSftpParam lastParams;
			FSSftpParam params = lastParams;

			if ( !GetSftpLogon( g_MainWin, params ) )
			{
				return false;
			}

			params.isSet = true;
			clPtr<FS> fs = new FSSftp( &params ) ;
			FSPath path( CS_UTF8, "/" );

			if ( !fs.IsNull() )
			{
				lastParams = params;
				p->LoadPath( fs, path, 0, 0, PanelWin::SET );
			}
		}
		break;
#endif

		case ID_DEV_TMP:
		{
			clPtr<FSTmp> fs = new FSTmp( OtherPanel->GetFS() );
			p->LoadPath( fs, FSTmp::rootPathName, 0, 0, PanelWin::PUSH );
		}
		break;
	};

	return false;
}

void SelectDriveDlg( PanelWin* p, PanelWin* OtherPanel )
{
	bool RedoDialog = false;

	do
	{
		RedoDialog = SelectDriveInternal( p, OtherPanel );
	}
	while ( RedoDialog );
}

void ReturnToDefaultSysDir()
{
#ifdef _WIN32
	wchar_t buf[4096] = L"";

	if ( GetSystemDirectoryW( buf, 4096 ) > 0 )
	{
		SetCurrentDirectoryW( buf );
	}

#else
	chdir( "/" );
#endif
}

void OpenHomeDir( PanelWin* p, PanelWin* OtherPanel )
{
#ifdef _WIN32
	std::vector<unicode_t> homeUri = GetHomeUriWin();

	if ( homeUri.data() )
	{
		const std::vector<clPtr<FS>> checkFS =
		{
			p->GetFSPtr(),
			OtherPanel->GetFSPtr()
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
	const sys_char_t* home = ( sys_char_t* ) getenv( "HOME" );
	if ( !home )
	{
		return;
	}

	FSPath path( sys_charset_id, home );
	p->LoadPath( new FSSys(), path, 0, 0, PanelWin::SET );
#endif
}
