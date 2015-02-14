/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#include <wal.h>

using namespace wal;

#include "string-util.h"
#include "ext-app.h"
#include "unicode_lc.h"
#include "strmasks.h"
#include <sys/types.h>
#include <dirent.h>
#include <sys/time.h>

#include <unordered_map>

//alias надо применить


/*
   как выяснилось (описания не нашел)

   /usr/share/mime/globs
      application/msword:*.doc
      text/plain:*.doc

   /usr/share/mime/globs2 ??? хрен знает зачем он
      75:application/pkcs8:*.p8
      55:application/x-pkcs12:*.pfx
      50:application/msword:*.doc
      50:text/plain:*.doc
      50:application/msword:*.doc
      10:text/x-readme:readme*
      10:text/x-makefile:makefile.*

   /usr/share/mime/aliases
      application/x-msword application/msword
      application/x-netscape-bookmarks application/x-mozilla-bookmarks

   /usr/share/mime/subclasses
      application/x-cdrdao-toc text/plain
      application/pkix-crl+pem application/x-pem-file
      application/x-ruby application/x-executable

   /usr/share/applications/defaults.list
      [Default Applications]
      application/csv=libreoffice-calc.desktop      !!! может содержать несколько файлов описаний приложений через ';' (в ming например)
      application/excel=libreoffice-calc.desktop
      application/msexcel=libreoffice-calc.desktop
      ...

*/


class MimeGlobs
{
	std::string fileName;
	time_t mtime;

	std::unordered_map< std::wstring, std::vector<int> > m_ExtMimeHash;

	void AddExt( const char* s, int m );

	struct MaskNode
	{
		std::vector<unicode_t> mask;
		int mime;
		MaskNode* next;
		MaskNode( const unicode_t* s, int m ): mask( new_unicode_str( s ) ), mime( m ), next( 0 ) { }
	};

	MaskNode* maskList;
	void AddMask( const char* s, int m );
	void ClearMaskList() { while ( maskList ) { MaskNode* p = maskList->next; delete maskList; maskList = p; } }
public:
	MimeGlobs( const char* fname ): fileName( new_char_str( fname ) ), mtime( 0 ), maskList( 0 ) { }
	int GetMimeList( const unicode_t* fileName, ccollect<int>& list );
	void Refresh();
	~MimeGlobs();
};

class MimeSubclasses
{
	std::string fileName;
	time_t mtime;
	std::unordered_map< int, ccollect<int> > hash;
public:
	MimeSubclasses( const char* fn ): fileName( new_char_str( fn ) ), mtime( 0 ) { }
	int GetParentList( int mime, ccollect<int>& list );
	void Refresh();
	~MimeSubclasses();
};

class MimeAliases
{
	std::string fileName;
	time_t mtime;
	std::unordered_map<int, int> data;
public:
	MimeAliases( const char* fname ): fileName( new_char_str( fname ) ), mtime( 0 ) { }
	void Refresh();

	int Check( int mime )
	{
		auto i = data.find( mime );
		return i != data.end() ? i->second : mime;
	}
	~MimeAliases();
};

class MimeDB: public iIntrusiveCounter
{
	MimeGlobs globs;
	MimeSubclasses subclasses;
	MimeAliases aliases;
	void AddParentsRecursive( int mime, ccollect<int>* pList, std::unordered_map<int, bool>* pHash );
public:
	MimeDB( const char* location );
	void Refresh();
	int GetMimeList( const unicode_t* fileName, ccollect<int>& list );
	int CheckAlias( int id );
	~MimeDB();
};

class AppDefListFile: public iIntrusiveCounter
{
	std::string fileName;
	time_t mtime;
	std::unordered_map<int, ccollect<int, 1> > hash;
public:
	AppDefListFile( const char* fname ): fileName( new_char_str( fname ) ), mtime( 0 ) { }
	void Refresh();
	int GetAppList( int mime, ccollect<int>& appList );
	~AppDefListFile();
};


struct AppNode: public iIntrusiveCounter
{
	std::vector<unicode_t> name;
	std::vector<unicode_t> exec;
	bool terminal;
	AppNode(): terminal( true ) {}
};


class AppDB: public iIntrusiveCounter
{
	std::string appDefsPrefix;
	std::unordered_map<int, clPtr<AppNode> > apps;
	std::unordered_map<int, ccollect<int> > mimeMapHash;

	bool ReadAppDesctopFile( const char* fName );

	AppDefListFile defList;
public:
	AppDB( const char* prefix );

	int GetAppList( int mime, ccollect<int>& appList );

	void Refresh() { defList.Refresh(); }

	AppNode* GetApp( int id );

	~AppDB();
};




//добавляет в хэш имена выполняемых файлов из указанного каталога
static void SearchExe( const char* dirName, cstrhash<bool>& hash )
{
	DIR* d = opendir( dirName );

	if ( !d ) { return; }

	try
	{
		struct dirent ent, *pEnt;

		while ( true )
		{
			if ( readdir_r( d, &ent, &pEnt ) ) { goto err; }

			if ( !pEnt ) { break; }

			//skip . and ..
			if ( ent.d_name[0] == '.' && ( !ent.d_name[1] || ( ent.d_name[1] == '.' && !ent.d_name[2] ) ) )
			{
				continue;
			}

			std::string filePath = std::string(dirName) + "/" + std::string(ent.d_name);

			struct stat sb;

			if ( stat( filePath.data(), &sb ) ) { continue; }

			if ( ( sb.st_mode & S_IFMT ) == S_IFREG && ( sb.st_mode & 0111 ) != 0 )
			{
				hash[ent.d_name] = true;
			}
		};

err:
		closedir( d );
	}
	catch ( ... )
	{
		closedir( d );
		throw;
	}
}

static clPtr<cstrhash<bool> > pathExeList;

//определяет наличие исполняемого файла в каталогах в PATH
//списки кэшируются
bool ExeFileExist( const char* name )
{
	if ( name[0] == '/' ) //абсолютный путь
	{
		struct stat sb;

		if ( stat( name, &sb ) ) { return false; }

		return true;

		return  ( ( sb.st_mode & S_IFMT ) == S_IFREG && ( sb.st_mode & 0111 ) != 0 );
	}

	if ( !pathExeList.ptr() )
	{
		pathExeList = new cstrhash<bool>;
		const char* pl = getenv( "PATH" );

		if ( !pl ) { return false; }

		std::string paths = new_char_str( pl );
		char* s = paths.data();

		while ( *s )
		{
			char* t = s;

			while ( *t && *t != ':' ) { t++; }

			if ( *t )
			{
				*t = 0;
				t++;
			}

			SearchExe( s, *( pathExeList.ptr() ) );
			s = t;
		}
	}

	return pathExeList->exist( name ) != 0;
}


//#define MIMEDEBUG

#ifdef MIMEDEBUG
static std::unordered_map<int, std::string > mimeIdToNameHash;
const char* GetMimeName( int n )
{
	auto i = mimeIdToNameHash.find( n );
	return i != mimeIdToNameHash.end() ? i->second.data() : "";
}
#endif

static int GetMimeID( const char* mimeName )
{
	static cstrhash<int> hash;
	static int id = 0;

	int* pn = hash.exist( mimeName );

	if ( pn ) { return *pn; }

	id++;
	hash[mimeName] = id;

#ifdef MIMEDEBUG
	mimeIdToNameHash[id] = new_char_str( mimeName );
#endif
	return id;
}

static std::unordered_map<int, std::string > appIdToNameHash;

static int GetAppID( const char* appName )
{
	static cstrhash<int> hash;
	static int id = 0;

	int* pn = hash.exist( appName );

	if ( pn ) { return *pn; }

	id++;
	hash[appName] = id;

	appIdToNameHash[id] = new_char_str( appName );
	return id;
}

static const char* GetAppName( int id )
{
	auto i = appIdToNameHash.find( id );

	return ( i == appIdToNameHash.end() ) ? "" : i->second.data();
}


inline time_t GetMTime( const char* fileName )
{
	struct stat st;
	return stat( fileName, &st ) != 0 ? 0 : st.st_mtime;
}

static std::vector<unicode_t> GetFileExtLC( const unicode_t* uri )
{
	if ( !uri ) { return std::vector<unicode_t>(); }

	const unicode_t* ext = find_right_char<unicode_t>( uri, '.' );

	if ( !ext || !*ext ) { return std::vector<unicode_t>(); }

	std::vector<unicode_t> a = new_unicode_str( ext + 1 ); //пропустить точку
	unicode_t* s = a.data();

	for ( ; *s; s++ ) { *s = UnicodeLC( *s ); }

	return a;
}



////////////////// MimeGlobs //////////////////////////

inline char* SS( char* s ) { while ( IsSpace( *s ) ) { s++; } return s; }
inline char* SNotS( char* s ) { while ( *s && !IsSpace( *s ) ) { s++; } return s; }
inline char* SNotC( char* s, int c ) { while ( *s && *s != c ) { s++; } return s; }

inline void MimeGlobs::AddExt( const char* s, int m )
{
	unicode_t buf[100];

	int len = 0;

	for ( ; *s && len < 100 - 1; s++, len++ )
	{
		buf[len] = UnicodeLC( *s );
	}

	buf[len] = 0;

	m_ExtMimeHash[ std::wstring( buf ) ].push_back( m );
}


inline void MimeGlobs::AddMask( const char* s, int m )
{
	unicode_t buf[100];
	int len = 0;

	for ( ; *s && len < 100 - 1; s++, len++ )
	{
		buf[len] = UnicodeLC( *s );
	}

	buf[len] = 0;

	MaskNode* node = new MaskNode( buf, m );
	node->next = maskList;
	maskList = node;
}

void MimeGlobs::Refresh()
{
	time_t tim = GetMTime( fileName.data() );

	if ( tim == mtime ) { return; }

	mtime = tim;
	m_ExtMimeHash.clear();
	ClearMaskList();

	try
	{
		BFile f;
		f.Open( fileName.data() ); //"/usr/share/mime/glibs");
		char buf[4096];

		while ( f.GetStr( buf, sizeof( buf ) ) )
		{
			char* s = SS( buf );

			if ( *s == '#' ) { continue; } //комментарий

			//text/plain:*.doc

			char* e = SNotC( s, ':' );

			if ( !e ) { continue; }

			*e = 0;
			e++;
			e = SS( e );

			int mimeId = GetMimeID( s );

			if ( e[0] == '*' && e[1] == '.' )
			{
				e += 2;
				char* t = SNotS( e );

				if ( *t ) { *t = 0; }

				AddExt( e, mimeId );
			}
			else
			{
				char* t = SNotS( e );

				if ( *t ) { *t = 0; }

				AddMask( e, mimeId );
			}
		}
	}
	catch ( cexception* ex )
	{
		ex->destroy();
		return;
	}

	return;
}

int MimeGlobs::GetMimeList( const unicode_t* fileName, ccollect<int>& list )
{
	std::vector<unicode_t> ext = GetFileExtLC( fileName );

	if ( ext.data() )
	{
		auto iter = m_ExtMimeHash.find( ext.data() );

		if ( iter != m_ExtMimeHash.end() )
		{
			const std::vector<int>& p = iter->second;

			for ( size_t i = 0, cnt = p.size(); i < cnt; i++ )
			{
				list.append( p[ i ] );
			}
		}
	}

	{
		//пробег по маскам
		const unicode_t* fn = find_right_char<unicode_t>( fileName, '/' );

		if ( fn ) { fn++; }
		else { fn = fileName; }

		std::vector<unicode_t> str( unicode_strlen( fn ) + 1 );
		unicode_t* s = str.data();

		while ( *fn ) { *( s++ ) = UnicodeLC( *( fn++ ) ); }

		*s = 0;

		for ( MaskNode* p = maskList; p; p = p->next )
			if ( p->mask.data() && accmask( str.data(), p->mask.data() ) )
			{
				list.append( p->mime );
			}
	}

	return list.count();
}

MimeGlobs::~MimeGlobs() { ClearMaskList(); }



//////////////////////////// MimeSubclasses ////////////////////////////////

MimeSubclasses::~MimeSubclasses() {};

void MimeSubclasses::Refresh()
{
	time_t tim = GetMTime( fileName.data() );

	if ( tim == mtime ) { return; }

	mtime = tim;
	hash.clear();

	try
	{
		BFile f;
		f.Open( fileName.data() ); //"/usr/share/mime/subclasses");
		char buf[4096];

		while ( f.GetStr( buf, sizeof( buf ) ) )
		{
			char* s = SS( buf );

			if ( *s == '#' ) { continue; } //комментарий

			char* parent = SNotS( s );

			if ( !parent ) { continue; }

			*parent = 0;
			parent++;
			parent = SS( parent );
			char* t = SNotS( parent );

			if ( t ) { *t = 0; }

			hash[GetMimeID( s )].append( GetMimeID( parent ) );
		}
	}
	catch ( cexception* ex )
	{
		ex->destroy();
		return;
	}
}

int MimeSubclasses::GetParentList( int mime, ccollect<int>& list )
{
	auto i = hash.find( mime );

	ccollect<int>* p = ( i == hash.end() ) ? nullptr : &( i->second );

	if ( p )
	{
		for ( int i = 0, cnt = p->count(); i < cnt; i++ )
		{
			list.append( p->get( i ) );
		}
	}

	return list.count();
}


/////////////////// MimeAliases //////////////////////////////

void MimeAliases::Refresh()
{
	time_t tim = GetMTime( fileName.data() );

	if ( tim == mtime ) { return; }

	mtime = tim;
	data.clear();

	try
	{
		BFile f;
		f.Open( fileName.data() ); //"/usr/share/mime/aliases");
		char buf[4096];

		while ( f.GetStr( buf, sizeof( buf ) ) )
		{
			char* s = SS( buf );

			if ( *s == '#' ) { continue; } //комментарий

			char* p = SNotS( s );

			if ( !p ) { continue; }

			*p = 0;
			p++;
			p = SS( p );
			char* t = SNotS( p );

			if ( t ) { *t = 0; }

			data[GetMimeID( s )] = GetMimeID( p );
		}
	}
	catch ( cexception* ex )
	{
		ex->destroy();
		return;
	}
}

MimeAliases::~MimeAliases() {}


///////////////////////////// MimeDB //////////////////////////////////////////////////

MimeDB::MimeDB( const char* location )
	:  globs( carray_cat<char>( location, "globs" ).data() ),
	   subclasses( carray_cat<char>( location, "subclasses" ).data() ),
	   aliases( carray_cat<char>( location, "aliases" ).data() )
{
}


void MimeDB::AddParentsRecursive( int mime, ccollect<int>* pList, std::unordered_map<int, bool>* pHash )
{
	ccollect<int> parentList;

	if ( subclasses.GetParentList( mime, parentList ) )
	{
		int i;

		for ( i = 0; i < parentList.count(); i++ )
		{
			bool Exists = pHash->find( parentList[i] ) != pHash->end();

			if ( !Exists )
			{
				pList->append( parentList[i] );
				( *pHash )[ parentList[i] ] = true;

				AddParentsRecursive( parentList[i], pList, pHash );
			}
		}

//		for (i=0; i<parentList.count(); i++)
//			AddParentsRecursive(parentList[i], pList, pHash);
	}
}

void MimeDB::Refresh()
{
	globs.Refresh();
	subclasses.Refresh();
	aliases.Refresh();
}

int MimeDB::GetMimeList( const unicode_t* fileName, ccollect<int>& outList )
{
	ccollect<int> temp1;

	if ( globs.GetMimeList( fileName, temp1 ) > 0 )
	{

		std::unordered_map<int, bool> hash;
		int i;

		for ( i = 0; i < temp1.count(); i++ )
		{
			bool Exists = hash.find( temp1[i] ) != hash.end();

			if ( !Exists )
			{
				outList.append( temp1[i] );
				hash[ temp1[i] ] = true;

				AddParentsRecursive( temp1[i], &outList, &hash );
			}
		}

//		for (i=0; i<temp1.count(); i++)
//			AddParentsRecursive(temp1[i], &outList, &hash);
	}

	return outList.count();
}

MimeDB::~MimeDB() {}




/////////////////////// AppDefListFile ////////////////////////////////

AppDefListFile::~AppDefListFile() {}

int AppDefListFile::GetAppList( int mime, ccollect<int>& appList )
{
	auto i = hash.find( mime );

	ccollect<int, 1>* p = ( i == hash.end() ) ? nullptr : &( i->second );

	if ( p )
	{
		for ( int i = 0, cnt = p->count(); i < cnt; i++ )
		{
			appList.append( p->get( i ) );
		}
	}

	return appList.count();
}

inline void EraseLastSpaces( char* s )
{
	char* ls = 0;

	for ( ; *s; s++ ) if ( !IsSpace( *s ) ) { ls = s; }

	if ( ls ) { ls[1] = 0; }
}

inline int UCase( int c ) { return ( c >= 'a' && c <= 'z' ) ? c - 'a' + 'A' : c; }

static int CmpNoCase( const char* a, const char* b )
{
	for ( ; *a && *b; a++, b++ )
	{
		int ca = UCase( *a ), cb = UCase( *b );

		if ( ca != cb )
		{
			return ca < cb ? -1 : 1;
		}
	}

	return *a ? 1 : *b ? -1 : 0;
}


void AppDefListFile::Refresh()
{

	time_t tim = GetMTime( fileName.data() );

	if ( tim == mtime ) { return; }

	mtime = tim;
	hash.clear();

	try
	{
		BFile f;

		f.Open( fileName.data() );

		char buf[4096];
		bool readLine = false;

		while ( f.GetStr( buf, sizeof( buf ) ) )
		{
			EraseLastSpaces( buf );
			char* s = buf;
			s = SS( s );

			if ( *s == '[' )
			{
				readLine = !CmpNoCase( s, "[Default Applications]" ) ||
				           !CmpNoCase( s, "[Added Associations]" );
				continue;
			}

			if ( !readLine ) { continue; }

			char* mimeStr = s;

			while ( *s && !IsSpace( *s ) && *s != '=' ) { s++; }

			if ( IsSpace( *s ) )
			{

				*s = 0;
				s++;
				s = SS( s );
			}

			if ( *s != '=' )
			{
				continue;
			}

			*s = 0;
			s++;
			s = SS( s );

			int mimeId = GetMimeID( mimeStr );

			//в mint бывает несколько файлов описания приложений через ';' причем каких то файлов может не быть
			while ( *s )
			{

				char* t = SNotC( s, ';' );

				if ( *t )
				{
					*t = 0;
					t++;
				}

				hash[mimeId].append( GetAppID( s ) );
				s = t;
			}
		}

	}
	catch ( cexception* ex )
	{
		ex->destroy();
	}
}


//////////////////////////////// AppDB /////////////////////////////////////

AppDB::AppDB( const char* prefix )
	:  appDefsPrefix( new_char_str( prefix ) ),
	   defList( carray_cat<char>( prefix, "defaults.list" ).data() )
{
	DIR* d = opendir( prefix );

	if ( !d ) { return; }

	try
	{
		struct dirent ent, *pEnt;

		while ( true )
		{
			if ( readdir_r( d, &ent, &pEnt ) ) { goto err; }

			if ( !pEnt ) { break; }

			//skip . and ..
			if ( ent.d_name[0] == '.' && ( !ent.d_name[1] || ( ent.d_name[1] == '.' && !ent.d_name[2] ) ) )
			{
				continue;
			}

			ReadAppDesctopFile( ent.d_name );
		};

err:
		closedir( d );
	}
	catch ( cexception* ex )
	{
		closedir( d );
		ex->destroy();
		throw;
	}
	catch ( ... )
	{
		closedir( d );
		throw;
	}
}

AppNode* AppDB::GetApp( int id )
{
	auto i = apps.find( id );

	clPtr<AppNode>* p = ( i == apps.end() ) ? nullptr : &( i->second );

	if ( !p )
	{
		if ( !ReadAppDesctopFile( GetAppName( id ) ) ) { return 0; }

		i = apps.find( id );

		p = ( i == apps.end() ) ? nullptr : &( i->second );
	}

	return p ? p->ptr() : 0;
}

int AppDB::GetAppList( int mime, ccollect<int>& appList )
{
	defList.GetAppList( mime, appList );

	auto i = mimeMapHash.find( mime );

	ccollect<int>* p = ( i == mimeMapHash.end() ) ? nullptr : &( i->second );

	if ( p )
	{
		for ( int i = 0, cnt = p->count(); i < cnt; i++ )
		{
			appList.append( p->get( i ) );
		}
	}

	return appList.count();
}

AppDB::~AppDB() {}


bool AppDB::ReadAppDesctopFile( const char* name )
{
	int id = GetAppID( name );

	if ( apps.find( id ) != apps.end() ) { return true; }

	clPtr<AppNode> app = new AppNode();

	try
	{
		BFile f;
		f.Open( carray_cat<char>( appDefsPrefix.data(), name ).data() );

		char buf[4096];
		bool ok = false;

		bool application = false;

		ccollect<int> mimeList;

		while ( f.GetStr( buf, sizeof( buf ) ) )
		{
			EraseLastSpaces( buf );
			char* s = buf;
			s = SS( s );

			if ( *s == '[' )
			{
				ok = !CmpNoCase( s, "[Desktop Entry]" );
				continue;
			}

			if ( !ok ) { continue; }

			char* vname = s;

			while ( *s && !IsSpace( *s ) && *s != '=' ) { s++; }

			if ( IsSpace( *s ) )
			{
				*s = 0;
				s++;
				s = SS( s );
			}

			if ( *s != '=' )
			{
				continue;
			}

			*s = 0;
			s++;
			s = SS( s );

			if ( !CmpNoCase( vname, "TYPE" ) )
			{
				if ( !CmpNoCase( s, "APPLICATION" ) ) { application = true; }

				continue;
			}


			if ( !CmpNoCase( vname, "NAME" ) )
			{

				app->name = utf8_to_unicode( s );
				continue;
			}

			if ( !CmpNoCase( vname, "EXEC" ) )
			{
				app->exec = utf8_to_unicode( s );
				continue;
			}

			if ( !CmpNoCase( vname, "TRYEXEC" ) ) //проверяем сразу, чего каждый раз проверять
			{
				if ( !ExeFileExist( s ) )
				{
					apps[id] = 0;
					return 0;
				}

				continue;
			}

			if ( !CmpNoCase( vname, "TERMINAL" ) )
			{
				if ( !CmpNoCase( s, "TRUE" ) )
				{
					app->terminal = true;
				}
				else if ( !CmpNoCase( s, "FALSE" ) )
				{
					app->terminal = false;
				}

				continue;
			}

			if ( !CmpNoCase( vname, "MIMETYPE" ) )
			{
				while ( *s )
				{
					s = SS( s );
					char* m = s;

					while ( *s && *s != ';' ) { s++; }

					if ( *s == ';' ) { *s = 0; s++; }

					if ( *m )
					{
						mimeList.append( GetMimeID( m ) );
					}
				}

				continue;
			}
		}

		if ( !app->exec.data() || !application )
		{
			apps[id] = 0;
			return false;
		}

//		AppNode* pNode = app.ptr();

//printf("read %s - ok\n", name);

		apps[id] = app;

		for ( int i = 0; i < mimeList.count(); i++ )
		{
			mimeMapHash[mimeList[i]].append( id );
		}

		return true;

	}
	catch ( cexception* ex )
	{
		ex->destroy();
	}

	apps[id] = 0;
	return false;
}

///////////////////////////////////////

static clPtr<MimeDB> mimeDb;
static clPtr<AppDB> appDb;
static clPtr<AppDefListFile> userDefApp;

static bool FileIsExist( const char* s )
{
	struct stat sb;

	if ( stat( s, &sb ) ) { return false; }

	return ( sb.st_mode & S_IFMT ) == S_IFREG;
}

static bool DirIsExist( const char* s )
{
	struct stat sb;

	if ( stat( s, &sb ) ) { return false; }

	return ( sb.st_mode & S_IFMT ) == S_IFDIR;
}


static int _GetAppList( const unicode_t* fileName, ccollect<AppNode*>& list )
{
	if ( !mimeDb.ptr() )
	{
		if ( FileIsExist( "/usr/share/mime/globs" ) )
		{
			mimeDb = new MimeDB( "/usr/share/mime/" );
		}
		else
		{
			mimeDb = new MimeDB( "/usr/local/share/mime/" );
		}
	}

	if ( !appDb.ptr() )
	{
		if ( DirIsExist( "/usr/share/applications" ) )
		{
			appDb = new AppDB( "/usr/share/applications/" );
		}
		else
		{
			appDb = new AppDB( "/usr/local/share/applications/" );
		}
	}

	if ( !userDefApp.ptr() )
	{
		const char* home = getenv( "HOME" );

		if ( home )
		{
			userDefApp = new AppDefListFile( carray_cat<char>( home, "/.local/share/applications/mimeapps.list" ).data() );
		}
	}

	mimeDb->Refresh();
	appDb->Refresh();

	if ( userDefApp.ptr() )
	{
		userDefApp->Refresh();
	}

	ccollect<int> mimeList;

	if ( mimeDb->GetMimeList( fileName, mimeList ) )
	{
		int i;
		std::unordered_map<int, bool> hash;

		for ( i = 0; i < mimeList.count(); i++ )
		{
			ccollect<int> appList;

			if ( userDefApp.ptr() )
			{
				userDefApp->GetAppList( mimeList[i], appList );
			}

			appDb->GetAppList( mimeList[i], appList );

			for ( int j = 0; j < appList.count(); j++ )
			{
				bool Exists = hash.find( appList[j] ) != hash.end();

				if ( !Exists )
				{
					hash[appList[j]] = true;

					AppNode* p = appDb->GetApp( appList[j] );

					if ( p && p->exec.data() )
					{
						list.append( p );
					}
				}
			}
		}
	}

	return list.count();
}


static std::vector<unicode_t> PrepareCommandString( const unicode_t* exec, const unicode_t* uri )
{
	if ( !exec || !uri ) { return std::vector<unicode_t>(); }

	std::vector<unicode_t> cmd;

	const unicode_t* s = exec;


	int uriInserted = 0;

	while ( *s )
	{
		for ( ; *s && *s != '%'; s++ ) { cmd.push_back( *s ); }

		if ( *s )
		{
			switch ( s[1] )
			{
				case 'f':
				case 'F':
				case 'u':
				case 'U':
				{
					cmd.push_back( '"' );

					for ( const unicode_t* t = uri; *t; t++ ) { cmd.push_back( *t ); }

					cmd.push_back( '"' );
					uriInserted++;
				}

				s += 2;
				break;

				default:
					cmd.push_back( '%' );
					s++;
			};
		}
	}

	if ( !uriInserted ) //все равно добавим
	{
		cmd.push_back( ' ' );
		cmd.push_back( '"' );

		for ( const unicode_t* t = uri; *t; t++ ) { cmd.push_back( *t ); }

		cmd.push_back( '"' );
	}

	cmd.push_back( 0 );
	return cmd;
}


std::vector<unicode_t> GetOpenCommand( const unicode_t* uri, bool* needTerminal, const unicode_t** pAppName )
{
	ccollect<AppNode*> list;
	_GetAppList( uri, list );

	if ( list.count() > 0 )
	{
		if ( needTerminal ) { *needTerminal = list[0]->terminal; }

		if ( pAppName ) { *pAppName = list[0]->name.data(); }

		return PrepareCommandString( list[0]->exec.data(), uri );
	}

	return std::vector<unicode_t>();
}


clPtr<AppList> GetAppList( const unicode_t* uri )
{
	ccollect<AppNode*> list;
	_GetAppList( uri, list );

	if ( !list.count() ) { return 0; }

	clPtr<AppList> ret = new AppList();

	{
		//open item
		AppNode* p = list[0];

		AppList::Node node;
		node.terminal = p->terminal;

		static unicode_t ustr1[] = {'O', 'p', 'e', 'n', ' ', '(', 0};
		static unicode_t ustr2[] = {')', 0};

		node.name = carray_cat<unicode_t>( ustr1, p->name.data(), ustr2 );
		node.cmd = PrepareCommandString( p->exec.data(), uri );
		ret->list.append( node );
	}


	clPtr<AppList> openWith = new AppList();

	for ( int i = 1; i < list.count(); i++ )
	{
		AppList::Node node;
		node.terminal = list[i]->terminal;
		node.name = new_unicode_str( list[i]->name.data() );
		node.cmd = PrepareCommandString( list[i]->exec.data(), uri );
		openWith->list.append( node );
	}

	if ( openWith->Count() > 0 )
	{
		AppList::Node node;
		static unicode_t openWidthString[] = { 'O', 'p', 'e', 'n', ' ', 'w', 'i', 't', 'h', 0};
		node.name = new_unicode_str( openWidthString );
		node.sub = openWith;
		ret->list.append( node );
	}


	return ret->Count() ? ret : 0;
}


