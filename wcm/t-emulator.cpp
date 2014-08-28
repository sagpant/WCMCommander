#include "t-emulator.h"
#include <swl.h>

static unicode_t decGraphic[] =
{
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
	0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
	0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
	0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
	0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x25ae,
	0x25c6, 0x2592, 0x2409, 0x240c, 0x240d, 0x240a, 0x00b0, 0x00b1,
	0x2424, 0x240b, 0x2518, 0x2510, 0x250c, 0x2514, 0x253c, 0x23ba,
	0x23bb, 0x2500, 0x23bc, 0x23bd, 0x251c, 0x2524, 0x2534, 0x252c,
	0x2502, 0x2264, 0x2265, 0x03c0, 0x2260, 0x00a3, 0x00b7, '?'
};

static unicode_t* GetCSetTable( char c )
{
//надо сделать и вписать таблицы
	switch ( c )
	{
		case '0':
			return decGraphic;

		case 'A':
			return 0; //UK

		case 'B':
			return 0; //USASCII

		case '4':
			return 0; //Dutch

		case 'C':
			return 0; //Finish

		case '5':
			return 0;

		case 'R':
			return 0; //French

		case 'Q':
			return 0; //French Canadian

		case 'K':
			return 0; //Germab

		case 'Y':
			return 0; //Italic

		case 'E':
			return 0; //Norvegian Danish

		case '6':
			return 0;

		case 'Z':
			return 0; //Spanish

		case 'H':
			return 0; //Swedish

		case '7':
			return 0;

		case '=':
			return 0; //Swiss
	}

	return 0;
}



enum STATES
{
   ST_NORMAL = 0,
   ST_ESCAPE,  //esc
   ST_CSI,  //esc[
   ST_TP_NUM,  //esc]
   ST_TP_TEXT, //esc]<NUM>;
   ST_CHSET,  //esc(
   ST_DECPM,   //esc[p
   ST_G0_CHSET,
   ST_G1_CHSET,
   ST_G2_CHSET,
   ST_G3_CHSET,
   ST_DECSTR //esc!
};


void Emulator::ResetState()
{
	_state = ST_NORMAL;
	_attr.Reset();
	_scT = 0;
	_scB = _rows - 1;
	_screen = &_screen0;
	_utf8count = 0;
	_utf8char = ' ';
	_keypad = K_NORMAL;
	_wrap = true;
	_N.Reset();
};

#define INIT_ROWS 25
#define INIT_COLS 80

Emulator::Emulator()
	:  _screen0( 1024, INIT_COLS, &_clList ), _screen1( INIT_ROWS, INIT_COLS, &_clList ), _screen( &_screen0 ),
	   _rows( INIT_ROWS ), _cols( INIT_COLS ), _wrap( true )
{
	ResetState();
	_cursor.row = _rows - 1;
}

void Emulator::EraseDisplays()
{
	_screen0.Clear();
	_screen1.Clear();
}

int Emulator::SetSize( int r, int c )
{
	ASSERT( r >= 0 && c >= 0 );

	int s0Rows = r > 1024 ? r : 1024;
	_screen0.SetSize( s0Rows, c );
	_screen1.SetSize( r, c );

	int cdelta = _rows - _cursor.row;
	_rows = r;
	_cols = c;
	_scT = 0;
	_scB = r - 1;
	_clList.SetSize( s0Rows );
	_clList.SetAll( true );
	_cursor.row = _rows - cdelta;

	WinThreadSignal( 1 );
	return 0;
}

void Emulator::ScrollUp( int n )
{
	int b = _scT == 0 ? _screen->Rows() - 1 : _rows - _scT - 1;
	int a = _rows - _scB - 1 ;
//printf("a=%i b=%i\n", a, b);
	_screen->ScrollUp( a, b, n, _attr.Color() + ' ' );
}

void Emulator::ScrollDown( int n )
{
//printf("Emulator::ScrollDown (%i)\n", n);
	int b = _scT == 0 ? _screen->Rows() - 1 : _rows - _scT - 1;
	int a = _rows - _scB - 1;
	_screen->ScrollDown( a, b, n, _attr.Color() + ' ' );
}


void Emulator::AddUnicode( unicode_t ch )
{
	if ( _cursor.col >= _cols )
	{
		CR();
		LF();
	}

	_screen->SetLineChar( _rows - _cursor.row - 1, _cursor.col, ch | _attr.Color() );

	if ( _cursor.col < _cols ) { _cursor.col++; }
}

void Emulator::AddCh( char ch )
{
	if ( _utf8count > 0 )
	{
		if ( ( ch & ( 0x80 | 0x40 ) ) == 0x80 )
		{
			_utf8char = ( _utf8char << 6 ) + ( ch & 0x3F );
			_utf8count--;
		}
		else { _utf8count = 0; }

		if ( _utf8count != 0 ) { return; }
	}
	else  if ( ( ch & 0xE0 ) == 0xC0 )
	{
		_utf8char = ( ch  & 0x1F );
		_utf8count = 1;
		return;
	}
	else   if ( ( ch & 0xF0 ) == 0xE0 )
	{
		_utf8char = ( ch  & 0x0F );
		_utf8count = 2;
		return;
	}
	else
	{
		_utf8char = _attr.GetSymbol( ch );
	}

	AddUnicode( _utf8char );
}

void Emulator::InternalPrint( const unicode_t* str, unsigned fg, unsigned bg )
{
	unsigned savedFg = _attr.fColor;
	unsigned savedBg = _attr.bColor;
	_attr.fColor = fg;
	_attr.bColor = bg;

	for ( ; *str; str++ )
		switch ( *str )
		{
			case '\n':
				CR();
				LF();
				break;

			case '\t':
				Tab();
				break;

			default:
				AddUnicode( *str );
				break;
		}

	_attr.fColor = savedFg;
	_attr.bColor = savedBg;
}



void Emulator::CR()
{
	_cursor.col = 0;
	Changed( _cursor.row );
}

void Emulator::LF()
{
	if ( _cursor.row == _scB && _wrap )
	{
//printf("LF scroll top = %i, bottom = %i, row = %i\n", _scT, _scB, _cursor.row);
		ScrollUp( 1 );
	}
	else if ( _cursor.row < _rows + 1 )
	{
		Changed( _cursor.row );
		_cursor.row++;
		Changed( _cursor.row );
	}
}

void Emulator::Reset( bool clearScreen )
{
	ResetState();

	if ( clearScreen )
	{
		EraseDisplays();
	}

	_cursor.col = 0;
	_cursor.row = _rows - 1;
	_scT = 0;
	_scB = _rows - 1;
}


void Emulator::Tab()
{
	int pos = ( ( _cursor.col + _attr.tabSize ) / _attr.tabSize ) * _attr.tabSize;

	if ( pos < _cols )
	{
		Changed( _cursor.row );
		_screen->SetLineChar( _rows - _cursor.row - 1, _cursor.col, pos - _cursor.col, _attr.Color() + ' ' );
		_cursor.col = pos;
	}
	else
	{
		if ( _wrap )
		{
			_screen->SetLineChar( _rows - _cursor.row - 1, _cursor.col, _cols - _cursor.col,  _attr.Color() + ' ' );
			LF();
			_cursor.col = ( _cursor.col > 0 ) ? pos % _cursor.col : 0;
			_screen->SetLineChar( _rows - _cursor.row - 1, 0, _cursor.col,  _attr.Color() + ' ' );
		}
	}
}

void Emulator::RI()
{
	if ( _cursor.row == _scT )
	{
//printf("RI scroll top = %i, bottom = %i, row = %i\n", _scT, _scB, _cursor.row);
		ScrollDown( 1 );
	}
	else if ( _cursor.row > 0 )
	{
		Changed( _cursor.row );
		_cursor.row--;
		Changed( _cursor.row );
	}
}

void Emulator::IND()
{
//printf("IND\n");
	LF(); //temp
}

void Emulator::SetCursor( int r, int c )
{
	Changed( _cursor.row );

	if ( r >= _rows ) { r = _rows - 1; }

	if ( r < 0 ) { r = 0; }

	if ( c > _cols ) { c = _cols; }

	if ( c < 0 ) { c = 0; }

	_cursor.Set( r, c );
	Changed( _cursor.row );
}

inline void Emulator::IncCursor( int dr, int dc )
{
	Emulator::SetCursor( _cursor.row + dr, _cursor.col + dc );
}

void Emulator::RestoreCursor()
{
	Changed( _cursor.row );
	_cursor = _savedCursor;

	if ( _cursor.col > _cols ) { _cursor.col = _cols; }

	if ( _cursor.row >= _rows ) { _cursor.row = _rows - 1; }

	if ( _cursor.row < 0 ) { _cursor.row = 0; }

	Changed( _cursor.row );
}

void Emulator::EraseDisp( int mode )
{
	int i;

	switch ( mode )
	{
		case 0:
			for ( i = 0; i <= _rows - _cursor.row - 1; i++ ) { _screen->SetLineChar( i, 0, _cols, _attr.Color() + ' ' ); }

			break;

		case 1:
			for ( i = _rows - _cursor.row - 1; i < _rows; i++ ) { _screen->SetLineChar( i, 0, _cols, _attr.Color() + ' ' ); }

			break;

		case 2:
			for ( i = 0; i < _rows; i++ ) { _screen->SetLineChar( i, 0, _cols, _attr.Color() + ' ' ); }

			break;
	}
}

#define  DBG dbg_printf
//#define  DBG printf

void Emulator::Append( char ch )
{
	switch ( _state )
	{
		case ST_NORMAL:
			switch ( ch )
			{
				case 0:
					return; //NUL - ignore

				case 1:
					return;

				case 5:
					return; //ENQ

				case 7:
					return; //bell

				case 8:    //backspace
					if ( _cursor.col > 0 )
					{
						_cursor.col--;
						Changed( _cursor.row );
					}

					return;

				case 9:
					Tab();
					return;

				case 0xA:
					LF();
					return;

				case 0xB:
					LF();
					return;

				case 0xC:
					return; //FF

				case 0xD:
					CR();
					return;

				case 0x1B:
					_state = ST_ESCAPE;
					return;

				default:
					AddCh( ch );
					return;
			};

			break;

		case ST_ESCAPE:
			switch ( ch )
			{
				case '[':
					_state = ST_CSI;
					_N.Reset();
					return;

				case '(':
					_state = ST_G0_CHSET;
					return;

				case ')':
					_state = ST_G1_CHSET;
					return;

				case '*':
					_state = ST_G2_CHSET;
					return;

				case '+':
					_state = ST_G3_CHSET;
					return;

				case ']':
					_state = ST_TP_NUM;
					_N.Reset();
					return;

//ЭТО НЕВЕРНО
//		case '=': _keypad = K_APPLICATION; break;
//		case '>': _keypad = K_NORMAL; break;


				case '\\':
					DBG( "DBG: esc \\ (ST)\n" );
					break;

				case '^':
					DBG( "DBG: esc ^ (PM)\n" );
					break;

				case '_':
					DBG( "DBG: esc _ (APC)\n" );
					break;

				case '7':
					_savedCursor = _cursor;
					break;

				case '8':
					RestoreCursor();
					break;

				case 'C':
					Reset( true );
					break;

				case 'D':
					IND();
					break;

				case 'E':
					CR();
					LF();
					break;

				case 'H':
					DBG( "DBG: esc H (HTS)\n" );
					break;

				case 'M':
					RI();
					break;

				case 'N':
					DBG( "DBG: esc N (SS2)\n" );
					break;

				case 'O':
					DBG( "DBG: esc O (SS3)\n" );
					break;

				case 'P':
					DBG( "DBG: esc P (DCS)\n" );
					break;

				case 'V':
					DBG( "DBG: esc V (SPA)\n" );
					break;

				case 'W':
					DBG( "DBG: esc W (EPA)\n" );
					break;

				case 'X':
					DBG( "DBG: esc X (SOS)\n" );
					break;

				case 'Z':
					DBG( "DBG: esc Z (Terminal ID (abs))\n" );
					break;

				default:
					DBG( "unknown esc'%c'\n", ch );
			}

			break;

		case ST_TP_NUM:
			if ( _N.Read( ch ) ) { return; }

			if ( ch == ';' )
			{
				_state = ST_TP_TEXT;
				_TXT.clear();
				return;
			}

			break;

		case ST_TP_TEXT:
			if ( ch >= 0x20 )
			{
				_TXT.append( ch );
				return;
			}

			if ( ch == 7 )
			{
				//!!! set text parameter (_N[0], _TXT)
			}

			break;


		case ST_CSI:

			switch ( ch )
			{
				case '?':
					_state = ST_DECPM;
					_N.Reset();
					return;
			};

			if ( _N.ReadList( ch ) ) { return; }

			switch ( ch )
			{
				case 'd':
					SetCursor( ( _N[0] ? _N[0] : 1 ) - 1, _cursor.col );
					break;

				case 'f':
					SetCursor( ( _N[0] ? _N[0] : 1 ) - 1, ( _N[1] ? _N[1] : 1 ) - 1 );
					break;

				case 'r':
				{
					int top = ( _N[0] ? _N[0] : 1 ) - 1;
					int bottom = ( _N[1] ? _N[1] : _rows ) - 1;

					if ( top > bottom || bottom >= _rows || top < 0 ) { break; }

					_scT = top;
					_scB = bottom;
				}
				break;

				case 'm':
				{
					for ( int i = 0; i <= _N.count; i++ )
					{
//printf("_N[%i] = %i\n", i, _N[i]);
						switch ( _N[i] )
						{
							case 0:
								_attr.SetNormal();
								break;

							case 1:
								_attr.bold = true;
								break;

							case 4:
								_attr.underscore = true;
								break;

							case 5:
								_attr.blink = true;
								break;

							case 7:
								_attr.inverse = true;
								break;

								//case 8: set invisible
							case 22:
								_attr.bold = false;
								break;

							case 24:
								_attr.underscore = false;
								break;

							case 25:
								_attr.blink = false;
								break;

							case 27:
								_attr.inverse = false;
								break;

								//case 28: set visible
							case 30:
								_attr.fColor = 0;
								break;

							case 31:
								_attr.fColor = 4;
								break;

							case 32:
								_attr.fColor = 2;
								break;

							case 33:
								_attr.fColor = 6;
								break;

							case 34:
								_attr.fColor = 1;
								break;

							case 35:
								_attr.fColor = 5;
								break;

							case 36:
								_attr.fColor = 3;
								break;

							case 37:
								_attr.fColor = 7;
								break;

							case 39:
								_attr.fColor = DEF_FG_COLOR;
								break;

							case 40:
								_attr.bColor = 0;
								break;

							case 41:
								_attr.bColor = 4;
								break;

							case 42:
								_attr.bColor = 2;
								break;

							case 43:
								_attr.bColor = 6;
								break;

							case 44:
								_attr.bColor = 1;
								break;

							case 45:
								_attr.bColor = 5;
								break;

							case 46:
								_attr.bColor = 3;
								break;

							case 47:
								_attr.bColor = 7;
								break;

							case 49:
								_attr.bColor = DEF_BG_COLOR;
								break;

							default:
								DBG( "unknown %i in esc[%im\n", _N[i], _N[i] );
						}
					}
				}
				break;

				case '!':
					_state = ST_DECSTR;
					return;

				case '@':
					DBG( "DEBUG: esc %c (insert blank characters)\n", '@' );
					break;

				case 'A':
					IncCursor( - ( _N[0] ? _N[0] : 1 ), 0 );
					break;

				case 'B':
					IncCursor(   ( _N[0] ? _N[0] : 1 ), 0 );
					break;

				case 'C':
					IncCursor( 0, _N[0] ? _N[0] : 1 );
					break;

				case 'D':
					IncCursor( 0, -( _N[0] ? _N[0] : 1 ) );
					break;

				case 'E': //cursor - next n lines
					SetCursor( _cursor.row - _N[0] ? _N[0] : 1, 0 );
					break;

				case 'F': //cursor - prev n lines
					SetCursor( _cursor.row - _N[0] ? _N[0] : 1, 0 );
					break;

				case 'G': //set cursor column
					SetCursor( _cursor.row, ( _N[0] ? _N[0] : 1 ) - 1 );
					break;

				case 'H':
					SetCursor( ( _N[0] ? _N[0] : 1 ) - 1, ( _N[1] ? _N[1] : 1 ) - 1 );
					break;

				case 'I': //n Tabs
				{
					for ( int i = 0, n = _N[0] ? _N[0] : 1; i < n; i++ ) { Tab(); }
				}
				break;

				case 'J':
					EraseDisp( _N[0] );
					break;

				case 'K': // clear line
				{
//printf("Clear line (%i)\n", _N[0]);
					int from  = ( _N[0] > 0 ) ? 0 : _cursor.col;
					int to = ( _N[0] != 1 ) ? _cols : _cursor.col;
					_screen->SetLineChar( _rows - _cursor.row - 1, from, to - from, _attr.Color() + ' ' );
				}
				break;

				case 'L': // ins n lines (IL)
				{
//printf("IL\n");
					int count = _N[0] ? _N[0] : 1;
					_screen->ScrollDown( _rows - _scB - 1, _rows - _cursor.row - 1,  count, _attr.Color() + ' ' );
				}
				break;

				case 'M': // delete n lines (DL)
				{
//printf("DL\n");
					int count = _N[0] ? _N[0] : 1;
					_screen->ScrollUp( _rows - _scB - 1, _rows - _cursor.row - 1, count, _attr.Color() + ' ' );
				}

				break;


				case 'P': //Delete chars
				{
//printf("DC-\n");
					int count = _N[0] ? _N[0] : 1;
					_screen->DeleteLineChar( _rows - _cursor.row - 1, _cursor.col, count, _attr.Color() + ' ' );
				}
				break;

				case 'S': //scroll up n times
					ScrollUp( _N[0] ? _N[0] : 1 );
					break;

				case 'T': //scroll down n times
					ScrollDown( _N[0] ? _N[0] : 1 );
					break;

				case 'X': //erase n chars (ECH)
				{
//printf("ECH\n");
					_screen->SetLineChar( _rows - _cursor.row - 1, _cursor.col,  _N[0] ? _N[0] : 1, _attr.Color() + ' ' );
				}
				break;

				default:
					DBG( "unknown esc[%i'%c'\n", _N[0], ch );

			}

			break;

		case ST_DECPM:
			if ( _N.ReadList( ch ) ) { return; }

			switch ( ch )
			{
				case 'h':
				{
					for ( int i = 0; i <= _N.count; i++ )
						switch ( _N[i] )
						{
							case 1:
								_keypad = K_APPLICATION;
								break;

							case 7:
								_wrap = true;
								break;

							case 12:
								_attr.cursorBlinked = true;
								break;

							case 25:
								_attr.cursorVisible = true;
								break;

							case 1049:
								_savedCursor = _cursor;
								_screen1.Clear();

								//! no break
							case 47:
								_screen = &_screen1;
								_clList.SetAll( true );
								WinThreadSignal( 1 );
								break;

							default:
								DBG( "unknown %i in esc[?%ih\n", _N[i], _N[i] );
						};
				}
				break;

				case 'l':
				{
					for ( int i = 0; i <= _N.count; i++ )
						switch ( _N[i] )
						{
							case 1:
								_keypad = K_NORMAL;
								break;

							case 7:
								_wrap = false;
								break;

							case 12:
								_attr.cursorBlinked = false;
								break;

							case 25:
								_attr.cursorVisible = false;
								break;

							case 1049:
								RestoreCursor();

								//! no break
							case 47:
								_screen = &_screen0;
								_clList.SetAll( true );
								WinThreadSignal( 1 );
								break;

							default:
								DBG( "unknown %i in esc[?%il\n", _N[i], _N[i] );
						};
				}
				break;


				default:
					DBG( "unknown esc[?%i%c\n", _N[0], ch );
			}

			break;

		case ST_G0_CHSET:
			_attr.G[0] = GetCSetTable( ch );
			break;

		case ST_G1_CHSET:
			_attr.G[1] = GetCSetTable( ch );
			break;

		case ST_G2_CHSET:
			_attr.G[2] = GetCSetTable( ch );
			break;

		case ST_G3_CHSET:
			_attr.G[3] = GetCSetTable( ch );
			break;

		case ST_DECSTR:
			if ( ch == 'p' )
			{
				ResetState();
				EraseDisp( 2 );
				return;
			}

			DBG( "DBG: esc!%c\n", ch );
			break;



	};

	_state = ST_NORMAL;
}



////////////////////////////////////////// EmulatorScreen

EmulatorScreen::EmulatorScreen( int r, int c, EmulatorCLList* cl )
	: rows( r ), cols( c ), clList( cl )
{
	lineCount = r;

	if ( lineCount < 30 ) { lineCount = 30; }

	lineSize = c;

	if ( lineSize < 80 ) { lineSize = 80; }

	list.alloc( lineCount );

	for ( int i = 0; i < lineCount; i++ )
	{
		list[i].alloc( lineSize );
		ClearEmulatorLine( list[i].ptr(), lineSize );
	}
}

void EmulatorScreen::Clear()
{
	if ( cols > 0 )
		for ( int i = 0; i < rows; i++ )
		{
			ClearEmulatorLine( list[i].ptr(), lineSize );
		}
}

void EmulatorScreen::SetSize( int r, int c )
{
	if ( r > lineCount )
	{
		carray<carray<TermChar> > t( r );
		int i;
		int n = r - lineCount;

		for ( i = 0; i < n; i++ )
		{
			t[i].alloc( lineSize );
			ClearEmulatorLine( t[i].ptr(), lineSize );
		}

		for ( i = 0; i < lineCount; i++ )
		{
			t[i + n] = list[i];
		}

		lineCount = r;
		list = t;
	}

	if ( c > lineSize )
	{
		for ( int i = 0; i < lineCount; i++ )
		{
			carray<TermChar> p( c );

			if ( cols > 0 )
			{
				memcpy( p.ptr(), list[i].ptr(), cols * sizeof( TermChar ) );
			}

			ClearEmulatorLine( p.ptr() + cols, c - cols );
			list[i] = p;
		}

		lineSize = c;
	}

	cols = c;
	rows = r;
}

void EmulatorScreen::ScrollUp( int a, int b, int count, unsigned ch ) //a<=b
{

//printf("EmulatorScreen::ScrollUp a=%i, b=%i count=%i\n", a, b, count);

	a = CLN( a );
	b = CLN( b );

	if ( a > b ) { return; }

	int n = b - a;

	if ( n < count )
	{
		for ( int i = a; i <= b; i++ )
		{
			ClearEmulatorLine( list[i].ptr(), cols, ch );
		}

	}
	else
	{

		for ( ; count > 0; count-- )
		{
			carray<TermChar> t = list[b];
			ClearEmulatorLine( t.ptr(), cols, ch );

			for ( int i = b; i > a; i-- ) { list[i] = list[i - 1]; }

			list[a] = t;
		}
	}

	for ( int i = a; i <= b; i++ ) { SetCL( i ); }
}

void EmulatorScreen::ScrollDown( int a, int b, int count, unsigned ch ) //a<=b
{
//printf("EmulatorScreen::ScrollDown a=%i, b=%i count=%i\n", a, b, count);
	a = CLN( a );
	b = CLN( b );

	if ( a > b ) { return; }

	int n = b - a;

	if ( n < count )
	{
		for ( int i = a; i <= b; i++ ) { ClearEmulatorLine( list[i].ptr(), cols, ch ); }
	}
	else
	{
		for ( ; count > 0; count-- )
		{
			carray<TermChar> t = list[a];
			ClearEmulatorLine( t.ptr(), cols, ch );

			for ( int i = a; i < b; i++ ) { list[i] = list[i + 1]; }

			list[b] = t;
		}
	}

	for ( int i = a; i <= b; i++ ) { SetCL( i ); }
}

void EmulatorScreen::SetLineChar( int ln, int c, int count, unsigned ch )
{
//printf("EmulatorScreen::SetLineChar(ln %i, c %i, count %i, ch=%X\n",ln, c, count, ch );

	if ( ln >= rows || ln < 0 || c >= cols || c < 0 )
	{
		return;
	}

	if ( c + count > cols ) { count = cols - c; }

	TermChar* p = list[ln].ptr() + c;

	for ( ; count > 0; count-- ) { *( p++ ) = ch; }

	SetCL( ln );
}

void EmulatorScreen::SetLineChar( int ln, int c, unsigned ch )
{
	if ( ln >= rows || ln < 0 || c >= cols || c < 0 ) { return; }

//printf("Set(%i, %i) '%c'\n", ln, c, ch);
	list[ln][c] = ch;
	SetCL( ln );
}

void EmulatorScreen::InsertLineChar( int ln, int c, int count, unsigned ch )
{
	if ( ln >= rows || ln < 0 || c >= cols || c < 0 ) { return; }

	if ( c + count > cols ) { count = cols - c; }

	TermChar* p = list[ln].ptr();
	int shn = cols - ( c + count );

	if ( shn > 0 ) { memmove( p + c + count, p + c, shn * sizeof( TermChar ) ); }

	p += c;

	for ( ; count > 0; count-- ) { *( p++ ) = ch; }

	SetCL( ln );
}

void EmulatorScreen::DeleteLineChar( int ln, int c, int count, unsigned ch )
{
	if ( ln >= rows || ln < 0 || c >= cols || c < 0 ) { return; }

	if ( c + count > cols ) { count = cols - c; }

	TermChar* p = list[ln].ptr();
	int shn = cols - ( c + count );

	if ( shn > 0 ) { memmove( p + c, p + c + count, shn * sizeof( TermChar ) ); }

	p += c + shn;

	for ( ; count > 0; count-- ) { *( p++ ) = ch; }

	SetCL( ln );
}
