#ifndef swl_wincore_ui_inc_h
#define swl_wincore_ui_inc_h

namespace wal
{

////////////////////////////  Ui

	UiCache::UiCache(): updated( false ) {}
	UiCache::~UiCache() {}

	int GetUiID( const char* name )
	{
		static cstrhash<int> hash;
		static int id = 0;

		int* pn = hash.exist( name );

		if ( pn ) { return *pn; }

		id++;
		hash[name] = id;

//printf("%3i '%s'\n", id, name);

		return id;
	}

	class UiStream
	{
	public:
		UiStream() {}
		enum { EOFCHAR = EOF };
		virtual const char* Name();
		virtual int Next();
		virtual ~UiStream();
	};

	class UiStreamFile: public UiStream
	{
		std::vector<sys_char_t> _name;
		std::vector<char> _name_utf8;
		BFile _f;
	public:
		UiStreamFile( const sys_char_t* s ): _name( new_sys_str( s ) ), _name_utf8( sys_to_utf8( s ) ) { _f.Open( s ); }
		virtual const char* Name();
		virtual int Next();
		virtual ~UiStreamFile();
	};

	class UiStreamMem: public UiStream
	{
		const char* s;
	public:
		UiStreamMem( const char* data ): s( data ) {}
		virtual int Next();
		virtual ~UiStreamMem();
	};


	const char* UiStream::Name() { return ""; }
	int UiStream::Next() { return EOFCHAR; }
	UiStream::~UiStream() {}

	const char* UiStreamFile::Name() { return _name_utf8.data(); }
	int UiStreamFile::Next() { return _f.GetC(); }
	UiStreamFile::~UiStreamFile() {}

	int UiStreamMem::Next() { return *s ? *( s++ ) : EOF; }
	UiStreamMem::~UiStreamMem() {}


	struct UiParzerBuf
	{
		std::vector<char> data;
		int size;
		int pos;
		UiParzerBuf(): size( 0 ), pos( 0 ) {}
		void Clear() { pos = 0; }
		void Append( int c );
	};

	void UiParzerBuf::Append( int c )
	{
		if ( pos >= size )
		{
			int n = ( pos + 1 ) * 2;
			std::vector<char> p( n );

			if ( pos > 0 )
			{
				memcpy( p.data(), data.data(), pos );
			}

			data = p;
			size = n;
		}

		data[pos++] = c;
	}


///////////////////////////////// UiParzer
	inline bool IsAlpha( int c ) { return ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ); }
	inline bool IsDigit( int c ) { return c >= '0' && c <= '9'; }

	class UiParzer
	{
		UiStream* _stream;
		int _cc;
		int _tok;
		int NextChar();
		void SS() { while ( _cc >= 0 && _cc <= ' ' ) { NextChar(); } }
		UiParzerBuf _buf;
		int64 _i;
		int _cline;
	public:
		enum TOKENS
		{
		   TOK_ID = -500,   // /[_\a][_\a\d]+/
		   TOK_STR, // "...\?..."
		   TOK_INT,
		   TOK_EOF
		};

		void Syntax( const char* s = "" );
		UiParzer( UiStream* stream );
		int Tok() const { return _tok; }
		int64 Int() const { return _i; }
		const char* Str() { return _buf.data.data(); }
		int Next();
		void Skip( int tok );
		void SkipAll( int tok, int mincount = 1 );
		~UiParzer();
	};

	void UiParzer::Skip( int t )
	{
		if ( _tok != t )
		{
			char buf[64];
			sprintf( buf, "symbol not found '%c'", t );
			Syntax( buf );
		}

		Next();
	}

	void UiParzer::SkipAll( int t, int mincount )
	{
		int n;

		for ( n = 0; _tok == t; n++ ) { Next(); }

		if ( n < mincount )
		{
			char buf[64];
			sprintf( buf, "symbol not found '%c'", t );
			Syntax( buf );
		}
	}

	void UiParzer::Syntax( const char* s )
	{
		throw_msg( "Syntax error in '%s' line %i: %s", _stream->Name(), _cline + 1, s );
	}

	UiParzer::UiParzer( UiStream* stream )
		:  _stream( stream ),
		   _cc( 0 ),
		   _tok( 0 ),
		   _cline( 0 )
	{
		NextChar();
		Next();
	}

	UiParzer::~UiParzer() {}

	int UiParzer::NextChar()
	{
		if ( _cc == UiStream::EOFCHAR )
		{
			return _cc;
		}

		_cc = _stream->Next();

		if ( _cc == '\n' ) { _cline++; }

		return _cc;
	}



	int UiParzer::Next()
	{
begin:
		SS();

		if ( _cc == '_' || IsAlpha( _cc ) )
		{
			_buf.Clear();
			_buf.Append( _cc );
			NextChar();

			while ( IsAlpha( _cc ) || IsDigit( _cc ) || _cc == '_' || _cc == '-' )
			{
				_buf.Append( _cc );
				NextChar();
			};

			_buf.Append( 0 );

			_tok = TOK_ID;

			return _tok;
		}

		if ( _cc == '/' )
		{
			NextChar();

			if ( _cc != '/' )
			{
				_tok = '/';
				return _tok;
			}

			//comment
			while ( _cc != UiStream::EOFCHAR && _cc != '\n' )
			{
				NextChar();
			}

			goto begin;
		}

		if ( _cc == '"' || _cc == '\'' )
		{
			_buf.Clear();
			int bc = _cc;
			NextChar();

			while ( _cc != bc )
			{
				if ( _cc == UiStream::EOFCHAR )
				{
					Syntax( "invalid string constant" );
				}

				if ( _cc == '\\' )
				{
					NextChar();

					if ( _cc == UiStream::EOFCHAR )
					{
						Syntax( "invalid string constant" );
					}
				}

				_buf.Append( _cc );
				NextChar();
			}

			_buf.Append( 0 );
			_tok = TOK_STR;
			NextChar();
			return _tok;
		}

		if ( IsDigit( _cc ) )
		{
			int64 n = 0;

			if ( _cc == '0' )
			{
				NextChar();


				if ( _cc == 'x' || _cc == 'X' )
				{
					NextChar();

					while ( true )
					{
						if ( IsDigit( _cc ) )
						{
							n = n * 16 + ( _cc - '0' );
						}
						else if ( _cc >= 'a' && _cc <= 'f' )
						{
							n = n * 16 + ( _cc - 'a' ) + 10;
						}
						else if ( _cc >= 'A' && _cc <= 'F' )
						{
							n = n * 16 + ( _cc - 'A' ) + 10;
						}
						else
						{
							break;
						}

						NextChar();
					};

					_i = n;

					_tok = TOK_INT;

					return _tok;
				}
			}

			for ( ; IsDigit( _cc ); NextChar() )
			{
				n = n * 10 + ( _cc - '0' );
			}

			_i = n;
			_tok = TOK_INT;
			return _tok;
		};

		if ( _cc == UiStream::EOFCHAR )
		{
			_tok = TOK_EOF;
			return _tok;
		}

		_tok = _cc;
		NextChar();
		return _tok;
	}

	int64 UiValueNode::Int()
	{
		if ( !( flags & INT ) )
		{
			if ( !( flags & STR ) ) { return 0; }

			int64 n = 0;
			char* t = s.data();

			if ( !t || !*t ) { return 0; }

			bool minus = false;

			if ( *t == '-' )
			{
				minus = true;
				t++;
			}

			if ( *t == '+' ) { t++; }

			for ( ; *t >= '0' && *t <= '9'; t++ ) { n = n * 10 + ( *t - '0' ); }

			i = n;
			flags |= INT;
		}

		return i;
	}

	const char* UiValueNode::Str()
	{
		if ( !( flags & STR ) )
		{
			if ( !( flags & INT ) ) { return ""; }

			char buf[64];
			int_to_char<int64>( i, buf );
			s = new_char_str( buf );
			flags |= STR;
		}

		return s.data();
	}

	class UiValue;

	struct UiSelector
	{
		struct Node
		{
			int idC;
			int idN;
			bool oneStep;
			Node(): idC( 0 ), idN( 0 ), oneStep( true ) {}
			Node( int c, int n, bool o ): idC( c ), idN( n ), oneStep( o ) {}
		};

		struct CondNode
		{
			bool no;
			int id;
			CondNode(): no( false ), id( 0 ) {}
			CondNode( bool n, int i ): no( n ), id( i ) {}
		};

		struct ValueNode
		{
			int id;
			UiValue* data;
			ValueNode(): id( 0 ), data( 0 ) {}
			ValueNode( int i, UiValue* v ): id( i ), data( v ) {}
		};

		ccollect<Node> stack;
		int item;
		ccollect<CondNode> cond;
		ccollect<ValueNode> val;

		enum {CMPN = 4};
		unsigned char cmpLev[CMPN];

		int Cmp( UiSelector& a ) { int i = 0; while ( i < CMPN && cmpLev[i] == a.cmpLev[i] ) { i++; } return ( i >= CMPN ) ? 0 : cmpLev[i] < a.cmpLev[i] ? -1 : 1;  };

		void AppendObj( int c, int n, bool o )
		{
			stack.append( Node( c, n, o ) );

			if ( n ) { cmpLev[0]++; }

			if ( c ) { cmpLev[1]++; }
		}

		void AppendCond( bool no, int id ) { cond.append( CondNode( no, id ) ); cmpLev[3]++; }
		void AppendVal( int id, UiValue* v ) { val.append( ValueNode( id, v ) ); }
		void SetItem( int id ) {item = id; cmpLev[2] = id ? 1 : 0; }


		UiSelector(): item( 0 )   { for ( int i = 0; i < CMPN; i++ ) { cmpLev[i] = 0; } }

		void Parze( UiParzer& parzer );
	};

	void UiSelector::Parze( UiParzer& parzer )
	{
		while ( true )
		{
			bool oneStep = false;

			if ( parzer.Tok() == '>' )
			{
				oneStep = true;
				parzer.Next();
			}

			if ( parzer.Tok() == '*' )
			{
				parzer.Next();
				AppendObj( 0, 0, oneStep );
				continue;
			}

			int classId = 0;

			if ( parzer.Tok() == UiParzer::TOK_ID )
			{
				classId = GetUiID( parzer.Str() );
				parzer.Next();

			}

			int nameId = 0;

			if ( parzer.Tok() == '#' )
			{
				parzer.Next();

				if ( parzer.Tok() != UiParzer::TOK_ID )
				{
					parzer.Syntax( "object name not found" );
				}

				nameId = GetUiID( parzer.Str() );
				parzer.Next();
			}

			if ( !classId && !nameId ) { break; }

			AppendObj( classId, nameId, oneStep );
		}

		if ( parzer.Tok() == '@' ) //item
		{
			parzer.Next();

			if ( parzer.Tok() != UiParzer::TOK_ID ) { parzer.Syntax( "item name not found" ); }

			SetItem( item = GetUiID( parzer.Str() ) );
			parzer.Next();
		}

		while ( parzer.Tok() == ':' )
		{
			parzer.Next();
			bool no = false;

			if ( parzer.Tok() == '!' )
			{
				parzer.Next();
				no = true;
			}

			if ( parzer.Tok() != UiParzer::TOK_ID ) { parzer.Syntax( "condition name not found" ); }

			AppendCond( no, GetUiID( parzer.Str() ) );
			parzer.Next();
		}
	}

	bool UiValue::ParzeNode( UiParzer& parzer )
	{
		if ( parzer.Tok() == UiParzer::TOK_INT )
		{
			list.append( new UiValueNode( parzer.Int() ) );
		}
		else   if ( parzer.Tok() == UiParzer::TOK_STR )
		{
			list.append( new UiValueNode( parzer.Str() ) );
		}
		else
		{
			return false;
		}

		parzer.Next();
		return true;
	}

	void UiValue::Parze( UiParzer& parzer )
	{
		if ( parzer.Tok() == '(' )
		{
			parzer.Next();

			while ( true )
			{
				if ( !ParzeNode( parzer ) ) { break; }

				if ( parzer.Tok() != ',' ) { break; }

				parzer.Next();
			};

			parzer.Skip( ')' );
		}
		else
		{
			ParzeNode( parzer );
		}
	}

	UiValue::UiValue() {}
	UiValue::~UiValue() {}

	class UiRules
	{
		friend class UiCache;
		ccollect< cptr<UiSelector>, 0x100>  selectors;
		UiValue* values;
	public:
		UiRules(): values( 0 ) {}
		void Parze( UiParzer& parzer );
		~UiRules();
	};



	UiRules::~UiRules()
	{
		UiValue* v = values;

		while ( v )
		{
			UiValue* t = v;
			v = v->next;
			delete t;
		}
	}

	void UiRules::Parze( UiParzer& parzer )
	{
		while ( true )
		{
			parzer.SkipAll( ';', 0 );

			if ( parzer.Tok() == UiParzer::TOK_EOF ) { break; }

			ccollect<UiSelector*> slist;

			while ( true )
			{
				if ( parzer.Tok() == '{' ) { break; }

				cptr<UiSelector> sel = new UiSelector();
				slist.append( sel.ptr() );
				sel->Parze( parzer );

				int i = 0;

				while ( i < selectors.count() && sel->Cmp( *( selectors[i].ptr() ) ) < 0 ) { i++; }

				if ( i < selectors.count() )
				{
					selectors.insert( i, sel );
				}
				else
				{
					selectors.append( sel );
				}

				if ( parzer.Tok() != ',' ) { break; }

				parzer.Next();
			}

			parzer.Skip( '{' );

			if ( !slist.count() ) { parzer.Syntax( "empty list of selectors" ); }

			while ( true )
			{
				if ( parzer.Tok() != UiParzer::TOK_ID ) { break; }

				int id = GetUiID( parzer.Str() );
				parzer.Next();

				parzer.Skip( ':' );

				values = new UiValue( values );
				values->Parze( parzer );

				for ( int i = 0; i < slist.count(); i++ )
				{
					slist[i]->AppendVal( id, values );
				}

				if ( parzer.Tok() == '}' ) { break; }

				parzer.SkipAll( ';' );
			}

			parzer.Skip( '}' );
		}
	}

	void UiCache::Update( UiRules& rules, ObjNode* orderList, int orderlistCount )
	{
//printf("UiCache::Update 1: %i\n", orderlistCount);
		hash.clear();
		cptr<UiSelector>* psl = rules.selectors.ptr();
		int count = rules.selectors.count();

		for ( ; count > 0; count--, psl++ )
		{
			UiSelector* s = psl->ptr();
			int scount = s->stack.count();

			if ( scount > 0 )
			{
				if ( orderlistCount > 0 )
				{
					UiSelector::Node* sp = s->stack.ptr() + scount - 1;
					ObjNode* op = orderList;
					int n = orderlistCount;

					bool oneStep = true;

					while ( scount > 0 && n > 0 )
					{
						if ( ( !sp->idC || sp->idC == op->classId ) && ( !sp->idN || sp->idN == op->nameId ) )
						{
							oneStep = sp->oneStep;
							sp--;
							scount--;
							op++;
							n--;
						}
						else
						{
							if ( oneStep ) { break; }

							op++;
							n--;
						}
					}

					if ( scount <= 0 && s->val.count() )
					{
						int count = s->val.count();

//printf("'");
						for ( int i = 0; i < count; i++ )
						{
							hash[s->val[i].id].append( Node( s, s->val[i].data ) );
//printf("*");
						}

//printf("'\n");
					}
				}
			}
		}

//printf("cache hash count=%i\n", hash.count());
		updated = true;
	}


	UiValue* UiCache::Get( int id, int item, int* condList )
	{
		ccollect<Node>* ids = hash.exist( id );

		if ( !ids ) { return 0; }

		Node* p = ids->ptr();
		int count = ids->count();

		for ( ; count > 0; count--, p++ )
		{
			UiSelector* s = p->s;

			if ( !item && s->item ) { continue; }

			if ( item && s->item && s->item != item ) { continue; }

			int i;
			int n = s->cond.count();

			for ( i = 0; i < n; i++ )
			{
				int cid = s->cond[i].id;
				bool no = s->cond[i].no;

				bool found = false;

				if ( condList )
				{
					for ( int* t = condList; *t; t++ )
						if ( *t == cid )
						{
							found = true;
							break;
						}
				}

				if ( found == no ) { break; }
			}

			if ( i >= n ) { return p->v; }
		}

		return 0;
	}


	static cptr<UiRules> globalUiRules;

	static void ClearWinCache( WINHASHNODE* pnode, void* )
	{
		if ( pnode->win )
		{
			pnode->win->UiCacheClear();
		}
	}

	void UiReadFile( const sys_char_t* fileName )
	{
		UiStreamFile stream( fileName );
		UiParzer parzer( &stream );

		cptr<UiRules> rules  = new UiRules();
		rules->Parze( parzer );
		globalUiRules = rules;
		winhash.foreach( ClearWinCache, 0 );
	}

	void UiReadMem( const char* s )
	{
		UiStreamMem stream( s );
		UiParzer parzer( &stream );

		cptr<UiRules> rules  = new UiRules();
		rules->Parze( parzer );
		globalUiRules = rules;
		winhash.foreach( ClearWinCache, 0 );
	}

	void UiCondList::Set( int id, bool yes )
	{
		int i;

		for ( i = 0; i < N; i++ ) if ( buf[i] == id ) { buf[i] = 0; }

		if ( yes )
		{
			for ( i = 0; i < N; i++ )
				if ( !buf[i] )
				{
					buf[i] = id;
					return;
				}
		}
	}

	int uiColor = GetUiID( "color" );
	int uiBackground = GetUiID( "background" );
	int uiFocusFrameColor = GetUiID( "focus-frame-color" );
	int uiFrameColor = GetUiID( "frame-color" );
	int uiButtonColor = GetUiID( "button-color" );
	int uiMarkColor = GetUiID( "mark-color" );
	int uiMarkBackground = GetUiID( "mark-background" );
	int uiCurrentItem = GetUiID( "current-item" );
	int uiCurrentItemFrame = GetUiID( "current-item-frame-color" );
	int uiLineColor = GetUiID( "line-color" );
	int uiPointerColor = GetUiID( "pointer-color" );
	int uiOdd = GetUiID( "odd" );

	int uiEnabled = GetUiID( "enabled" );
	int uiFocus = GetUiID( "focus" );
	int uiItem = GetUiID( "item" );
	int uiClassWin = GetUiID( "Win" );

	unsigned Win::UiGetColor( int id, int itemId, UiCondList* cl, unsigned def )
	{
		if ( !globalUiRules.ptr() ) { return def; }

		int buf[0x100];
		int pos = 0;

		if ( cl )
		{
			for ( int i = 0; i < UiCondList::N && i < 0x100 - 10; i++ )
				if ( cl->buf[i] )
				{
					buf[pos++] = cl->buf[i];
				}
		}

		if ( IsEnabled() )
		{
			buf[pos++] = uiEnabled;
		}

		if ( InFocus() )
		{
			buf[pos++] = uiFocus;
		}

		buf[pos++] = 0;

//printf("1\n");
		if ( !uiCache.Updated() )
		{
//printf("2\n");
			//Update(UiRules &rules, ObjNode *orderList, int orderlistCount)
			ccollect<UiCache::ObjNode> wlist;

			for ( Win* w = this; w; w = w->Parent() )
			{
				wlist.append( UiCache::ObjNode( w->UiGetClassId(), w->UiGetNameId() ) );
			}

			uiCache.Update( *globalUiRules.ptr(), wlist.ptr(), wlist.count() );
		}

//printf("3\n");
		UiValue* v = uiCache.Get( id, itemId, buf );
//printf("4 %p\n", v);
//if (v) printf("5 %i\n", v->Int());
		return v ? v->Int() : def;
	};

	int Win::UiGetClassId()
	{
		return uiClassWin;
	}

} //namespace wal


#endif
