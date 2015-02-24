/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#include "ux_util.h"

#if !defined( _WIN32 )

#  include <wal.h>
#	include <signal.h>

using namespace wal;

static int  GetNWords( char* str, char** a, int n )
{
	int i = 0;
	char* s = str;
	char* t = str;

	while ( true )
	{
		while ( *s > 0 && *s <= ' ' ) { s++; } //пропустить пробелы

		if ( i < n ) { a[i] = s; i++; }

		while ( *t < 0 || *t > 32 ) { t++; } //пропустить НЕ пробелы

		if ( *t )
		{
			*t = 0;
			t++;
			s = t;
		}
		else { break; }
	}

	for ( int j = i; j < n; j++ ) //заполнить осталное пустыми строками
	{
		a[j] = t;
	}

	return i > n ? n : i;
}

/* пример /proc/mounts
rootfs / rootfs rw 0 0
sysfs /sys sysfs rw,nosuid,nodev,noexec,relatime 0 0
proc /proc proc rw,nosuid,nodev,noexec,relatime 0 0
udev /dev devtmpfs rw,relatime,size=1016976k,nr_inodes=254244,mode=755 0 0
devpts /dev/pts devpts rw,nosuid,noexec,relatime,gid=5,mode=620,ptmxmode=000 0 0
tmpfs /run tmpfs rw,nosuid,relatime,size=410312k,mode=755 0 0
/dev/disk/by-uuid/38e60133-3e05-427b-a944-770badcb7b43 / ext4 rw,relatime,errors=remount-ro,user_xattr,acl,barrier=1,data=ordered 0 0
none /sys/fs/fuse/connections fusectl rw,relatime 0 0
none /sys/kernel/debug debugfs rw,relatime 0 0
none /sys/kernel/security securityfs rw,relatime 0 0
none /run/lock tmpfs rw,nosuid,nodev,noexec,relatime,size=5120k 0 0
none /run/shm tmpfs rw,nosuid,nodev,relatime 0 0
binfmt_misc /proc/sys/fs/binfmt_misc binfmt_misc rw,nosuid,nodev,noexec,relatime 0 0
gvfs-fuse-daemon /home/wal/.gvfs fuse.gvfs-fuse-daemon rw,nosuid,nodev,relatime,user_id=1000,group_id=1000 0 0
/dev/sr0 /media/VBOXADDITIONS_4.1.22_80657 iso9660 ro,nosuid,nodev,relatime,uid=1000,gid=1000,iocharset=utf8,mode=0400,dmode=0500 0 0
*/


bool UxMntList( wal::ccollect< MntListNode >* pList )
{
	if ( !pList ) { return false; }

	try
	{

		BFile f;
		f.Open( ( sys_char_t* )"/proc/mounts" );

		char buf[4096];

		while ( f.GetStr( buf, sizeof( buf ) ) )
		{
			char* w[3];
			int n = GetNWords( buf, w, 3 );

			if ( n < 3 ) { continue; }

			if ( !strcmp( w[0], "none" ) ) { continue; }

			if ( !strcmp( w[1], "/" ) ) { continue; }

			if ( !strcmp( w[2], "tmpfs" ) ) { continue; }

			if ( !strcmp( w[2], "sysfs" ) ) { continue; }

			if ( !strcmp( w[2], "cgroup" ) ) { continue; }


			char* p = w[1];

			if ( p[0] == '/' && p[1] == 'p' && p[2] == 'r' && p[3] == 'o' && p[4] == 'c' && ( p[5] == '/' || !p[5] ) ) { continue; } // skip /proc[/...]

			if ( p[0] == '/' && p[1] == 'd' && p[2] == 'e' && p[3] == 'v' && ( p[4] == '/' || !p[4] ) ) { continue; } //skip /dev[/...]


			MntListNode node;
			node.path = w[1];
			node.type = w[2];
			pList->append( node );

		}
	}
	catch ( cexception* ex )
	{
		ex->destroy();
		return false;
	}

	return true;
}

void ExecuteDefaultApplication( const unicode_t* Path )
{
	if ( !fork() )
	{
		signal( SIGINT, SIG_DFL );
		static const char shell[] = "/bin/sh";
		std::string utf8 = unicode_to_utf8( Path );
		std::string command = "open \"" + utf8 + "\"";
		const char* params[] = { shell, "-c", command.c_str(), nullptr };

		execv( shell, ( char** ) params );

		exit( 1 );
	}
}

#endif
