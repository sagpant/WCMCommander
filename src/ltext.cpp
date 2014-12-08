/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#include "ltext.h"

#include <unordered_map>

using namespace wal;

static std::unordered_map< std::string, std::vector<char> > lText;

const char* LText( const char* index )
{
	const auto i = lText.find( index );

	if ( i == lText.end() ) return index;
	
	std::vector<char>* p = &( i->second );

	return p->data() ? p->data() : index;
}

const char* LText( const char* index, const char* def )
{
	const auto i = lText.find( index );

	if ( i == lText.end() ) return def;

	std::vector<char>* p = &( i->second );

	return p->data() ? p->data() : def;
}

inline char* SS( char* s )
{
	while ( *s > 0 && *s <= 0x20 ) { s++; }

	return s;
}

static bool AddStr( ccollect<char, 0x100>* a, char* s )
{
	while ( true )
	{
		s = SS( s );

		switch ( *s )
		{
			case 0:
				return true;

			case '"':
			{
				s++;

				while ( true )
				{
					if ( !*s || *s == '"' ) { break; }

					char c;

					if ( *s == '\\' && s[1] )
					{
						switch ( s[1] )
						{
							case 'n':
								c = '\n';
								break;

							case 'r':
								c = '\r';
								break;

							case 't':
								c = '\t';
								break;

							default:
								c = s[1];
								break;
						};

						s += 2;
					}
					else
					{
						c = *s;
						s++;
					}

					a->append( c );
				}

				if ( *s != '"' ) { return false; }

				s++;
			}
			break;

			case '#':
				return true;

			default:
				return false;
		}
	};
}


static void Add( ccollect<char, 0x100>& id, ccollect<char, 0x100>& txt )
{
	if ( id.count() > 0 && txt.count() > 0 )
	{
		id.append( 0 );
		txt.append( 0 );
		lText[id.ptr()] = new_char_str( txt.ptr() );
	}

	id.clear();
	txt.clear();
}

bool LTextLoad( sys_char_t* fileName )
{
	try
	{
		BFile f;
		f.Open( fileName );
		char buf[4096];
		ccollect<char, 0x100> id;
		ccollect<char, 0x100> txt;

		int mode = -1;

		while ( f.GetStr( buf, sizeof( buf ) ) )
		{
			char* s = buf;

			s = SS( s );

			if ( *s == '#' ) { continue; }

			if ( !*s )
			{
				Add( id, txt );
				mode = -1;
				continue;
			};

			if ( s[0] == 'i' && s[1] == 'd' )
			{
				mode = 0;
				s = SS( s + 2 );
				id.clear();
				txt.clear();

				if ( !AddStr( &id, s ) ) { id.clear(); }

				continue;
			}

			if ( s[0] == 't' && s[1] == 'x' && s[2] == 't' )
			{
				mode = 1;
				s = SS( s + 3 );
				txt.clear();

				if ( !AddStr( &txt, s ) ) { id.clear(); }

				continue;
			}

			if ( s[0] == '"' )
			{
				switch ( mode )
				{
					case 0:
						if ( !AddStr( &id, s ) ) { id.clear(); }

						break;

					case 1:
						if ( !AddStr( &txt, s ) ) { id.clear(); }

						break;
						//else - ignored
				};

				continue;
			}
		}

		Add( id, txt );

	}
	catch ( cexception* ex )
	{
		ex->destroy();
		return false;
	}

	return true;
}
