#ifdef USEFREETYPE

#include <unordered_map>

namespace FTU
{

	Mutex mutex;
	static int libState = -1;
	static FT_Library  library = 0;

	inline bool CheckLib()
	{
		if ( libState < 0 )
		{
			libState = FT_Init_FreeType( &library ) ? 1 : 0;
		}

		return libState == 0;
	}

	struct CharInfo
	{
		bool isSpace;
		int pxWidth;

		CharInfo( FT_GlyphSlotRec* p )
		{
			pxWidth = ( ( p->metrics.horiAdvance + 0x3F ) >> 6 );
			isSpace = !p->bitmap.buffer || p->bitmap.width <= 0 || p->bitmap.rows <= 0;
		};
		CharInfo() {}
	};


//кэш изображений (битмапов)
	class ImCache
	{
		enum CONST { MAX_IMAGE_WIDTH = 640, MAX_IMAGE_HEIGHT = 480 };

		struct NK
		{
			unsigned bg; //цвет фона
			unsigned fg; //цвет символа
			unicode_t ch; //символ

			operator unsigned() const { return bg + fg + ch; }

			//пиздец если саму функцию не описать как конснантную !!!
			bool operator == ( const NK& a ) const { return ch == a.ch && bg == a.bg && fg == a.fg; }
		};

		struct XYWH
		{
			int x, y, w, h;
			XYWH() {}
			XYWH( int _x, int _y, int _w, int _h ): x( _x ), y( _y ), w( _w ), h( _h ) {}
			void Set( int _x, int _y, int _w, int _h ) { x = _x; y = _y; w = _w; h = _h; }
		};

		struct Node
		{
			NK k;
			const NK& key() const { return k; }
			XYWH place;
		};

		chash<Node, NK> hash;

		SCImage image;
		int xPos;
		int yPos;
		int cHeight;

		Node* Alloc( unicode_t ch, unsigned bg, unsigned fg, int charW, int charH );
		void CopyArea( wal::GC& gc, Node* node, int x, int y );

		void DrawImage( wal::GC& gc,  Image32& im, int x, int y );

	public:
		ImCache();
		void Clear();

		//нарисовать если есть в кэше
		bool DrawIfExist( wal::GC& gc, int x, int y, unicode_t ch, unsigned bg, unsigned fg );

		//нарисовать и поместить в кэш если получится
		void Draw( wal::GC& gc, int x, int y, unicode_t ch, unsigned bg, unsigned fg, Image32& im );

		~ImCache();
	};

	struct ColorData   //чтоб не считать постоянно наложения цветов фона и текста
	{
		bool changed;
		unsigned bg;
		unsigned fg;
		unsigned colors[0x100];
		char bools[0x100];

		ColorData(): changed( true ), bg( 0 ), fg( 0xFFFFFF ) {}
		void SetBg( unsigned c ) { if ( c != bg ) { bg = c; changed = true; } }
		void SetFg( unsigned c ) { if ( c != fg ) { fg = c; changed = true; } }
		void Prepare() { if ( changed ) { memset( bools, 0, 0x100 ); changed = false;  } }
		bool IsSet( int n ) { return bools[n]; }
		unsigned Get( int n ) { return colors[n]; }
		void Set( int n, unsigned c ) { bools[n] = 1; colors[n] = c; }
	};


	class FFace
	{
		FT_Face face;
		ColorData cData;
		ImCache imCache;

		int pxAscender;
		int pxHeight;
		int _size;

		std::unordered_map<unicode_t, CharInfo> ciHash;

		void Clear() { if ( face )  { FT_Done_Face( face ); face = 0; imCache.Clear(); ciHash.clear(); } }

		int SetSize( int size, int xRes, int yRes );
		bool MkImage( Image32& im, FT_GlyphSlot slot );
		void OutChar( wal::GC& gc, int* px, int* py, unicode_t c );
		void OutCharF( wal::GC& gc, int* px, int* py, unicode_t c );

	public:
		FFace();
		int Load( const char* fileName, int index, int size, int xRes = 100, int yRes = 100 );
		void OutText( wal::GC& gc, int x, int y, const unicode_t* text, int count );
		void OutTextF( wal::GC& gc, int x, int y, const unicode_t* text, int count );
		cpoint GetTextExtents( const unicode_t* text, int count );

		const char* Name() { return face ? face->family_name : 0; }
		const char* StyleName() { return face ? face->style_name : 0; }
		unsigned FaceFlags() { return face ? face->face_flags : 0; }
		unsigned StyleFlags() { return face ? face->style_flags : 0; }
		int Size() { return _size; }

		int PxHeight() { return pxHeight; }
		~FFace();
	};



////////////////////// ImCache //////////////////////////////////////////////////////////////////

	ImCache::ImCache(): xPos( 0 ), yPos( 0 ), cHeight( 0 ) {}
	void ImCache::Clear() { hash.clear(); xPos = 0; yPos = 0; cHeight = 0; }

	inline void  ImCache::CopyArea( wal::GC& gc, Node* p, int x, int y )
	{
		XCopyArea( display, image.GetXDrawable(), gc.GetXDrawable(), gc.XHandle(), p->place.x, p->place.y, p->place.w, p->place.h,  x, y );
	}

	bool ImCache::DrawIfExist( wal::GC& gc, int x, int y, unicode_t ch, unsigned bg, unsigned fg )
	{
		NK k;
		k.bg = bg;
		k.fg = fg;
		k.ch = ch;
		Node* p = hash.get( k );

		if ( !p ) { return false; }

		CopyArea( gc, p, x, y );

		return true;
	}

	ImCache::Node* ImCache::Alloc( unicode_t ch, unsigned bg, unsigned fg, int charW, int charH )
	{
//return 0;

		if ( charW >  MAX_IMAGE_WIDTH || charH > MAX_IMAGE_HEIGHT ) { return 0; }

		if ( image.Width() - xPos < charW )
		{
			xPos = 0;
			yPos += cHeight;
			cHeight = 0;
		}

		Node node;
		node.k.bg = bg;
		node.k.fg = fg;
		node.k.ch = ch;

		if ( image.Width() - xPos >= charW && image.Height() - yPos >= charH )
		{
			node.place.Set( xPos, yPos, charW, charH );
			xPos += charW;

			if ( cHeight < charH ) { cHeight = charH; }

			return hash.put( node );
		}

		hash.clear();

		int w = image.Width();
		int h = image.Height();

		w = ( w ? ( w * 2 ) : ( 16 * charW ) );
		h = ( h ? ( h * 2 ) : charH * 2 );

		if ( w > MAX_IMAGE_WIDTH ) { w = MAX_IMAGE_WIDTH; }

		if ( h > MAX_IMAGE_HEIGHT ) { h = MAX_IMAGE_HEIGHT; }

		if ( image.Width() != w || image.Height() != h )
		{
			image.Create( w, h );
		}

		node.place.Set( 0, 0, charW, charH );

		xPos = charW;
		yPos = 0;
		cHeight = charH;

		return hash.put( node );
	}

	inline void ImCache::DrawImage( wal::GC& gc, Image32& im, int x, int y )
	{
		IntXImage ximage( im );
		ximage.Put( gc, 0, 0, x, y, im.width(), im.height() );
	}


	void ImCache::Draw( wal::GC& gc, int x, int y, unicode_t ch, unsigned bg, unsigned fg, Image32& im )
	{
		Node* node = Alloc( ch, bg, fg, im.width(), im.height() );

		if ( node )
		{
			wal::GC imageGC( &image );
			imageGC.SetFillColor( bg );
			DrawImage( imageGC, im, node->place.x, node->place.y );
			CopyArea( gc, node, x, y );
			return;
		}

		DrawImage( gc, im, x, y );
		return;
	}

	ImCache::~ImCache() {}


////////////////////// FFace ///////////////////////////////////////////////////////////


	FFace::FFace(): face( 0 ), pxAscender( 0 ), pxHeight( 0 ), _size( 1 ) {}
	FFace::~FFace() { if ( face ) { FT_Done_Face( face ); } }

	int FFace::Load( const char* fileName, int index, int size, int xRes, int yRes )
	{
		MutexLock lock( &mutex );

		if ( !CheckLib() ) { return 1; }

		Clear();
		int e = FT_New_Face( library, fileName, index, &face );

		if ( e ) { return e; }

		e = SetSize( size, xRes, yRes );

		if ( e ) { Clear(); return e; }

		return 0;
	}

	int FFace::SetSize( int size, int xRes, int yRes )
	{
		imCache.Clear();
		_size = 1;
		int e = FT_Set_Char_Size(
		           face,    /* handle to face object           */
		           0,       /* char_width in 1/64th of points  */
		           size,   /* char_height in 1/64th of points */
		           xRes,     /* horizontal device resolution    */
		           yRes ) ;   /* vertical device resolution  */

		if ( !e && face && face->size )
		{
			pxAscender =  face->size->metrics.ascender >> 6;
			pxHeight = ( face->size->metrics.height + 0x3F ) >> 6;

		}
		else
		{
			pxAscender = 0;
			pxHeight = 1;
		}

		if ( !e ) { _size = size; }

		return e;
	}

//bg(1-a)+fg*a
	inline unsigned fff( int bg, int fg, int a, int a1 )
	{
		return ( ( ( bg & 0xFF ) * a1 + ( fg & 0xFF ) * a ) / 256 ) & 0xFF;
	}

	bool FFace::MkImage( Image32& im, FT_GlyphSlot slot )
	{
		if ( !slot->bitmap.buffer ) { return false; }


		int h = pxHeight;
		int w = ( ( slot->metrics.horiAdvance + 0x3F ) >> 6 );


		if ( w <= 0 || h <= 0 ) { return false; }

		im.alloc( w, h );

		unsigned bg = cData.bg;
		unsigned fg = cData.fg;
		cData.Prepare();

		int left = slot->bitmap_left;
		int top = pxAscender - slot->bitmap_top - 1;

		if ( top < 0 ) { top = 0; }

		int bottom = top + slot->bitmap.rows;
		int right = left + slot->bitmap.width;

		if ( top >= h || bottom <= 0 || left >= w || right <= 0 )
		{
			uint32_t* p = im.line( 0 );

			for ( int cnt = w * h; cnt > 0; cnt--, p++ )
			{
				*p = bg;
			}

			return true;
		}

		int leftBgCount = ( left > 0 ) ? ( left > w ? w : left ) : 0;
		int rightBgCount = ( right < w ) ? w - right : 0;
		int bitmapOffset = left < 0 ? -left : 0;

		if ( left < 0 ) { left = 0; }

		if ( right > w ) { right = w; }

		int bitmapN = right - left;




		for ( int i = 0; i < h; i++ )
		{
			uint32_t* dest = im.line( i );

			if ( i < top || i >= bottom )
			{
				for ( int cnt = w; cnt > 0; cnt--, dest++ ) { *dest = bg; }
			}
			else
			{
				int n;

				for ( n = leftBgCount ; n > 0; n--, dest++ ) { *dest = bg; }

				unsigned char* s = slot->bitmap.buffer + ( i - top ) * slot->bitmap.width + bitmapOffset;

				for ( n = bitmapN; n > 0; n--, dest++, s++ )
				{
					unsigned c = *s;

					unsigned color;

					if ( c >= 0xFF )
					{
						color = fg;
					}
					else if ( c <= 0 )
					{
						color = bg;
					}
					else if ( cData.IsSet( c ) )
					{
						color = cData.Get( c );
					}
					else
					{
						// bg(1-a)+fg*a

						unsigned c1 = 255 - c;

						/*
						color = fff(bg, fg , c, c1) +
						(fff(bg>>8, fg>>8, c, c1)<<8)+
						(fff(bg>>16, fg>>16, c, c1)<<16);
						*/


						color =
						   ( ( ( ( bg & 0x0000FF ) * c1 + ( fg & 0x0000FF ) * c ) >> 8 ) & 0x0000FF ) +
						   ( ( ( ( bg & 0x00FF00 ) * c1 + ( fg & 0x00FF00 ) * c ) >> 8 ) & 0x00FF00 ) +
						   ( ( ( ( bg & 0xFF0000 ) * c1 + ( fg & 0xFF0000 ) * c ) >> 8 ) & 0xFF0000 );



						cData.Set( c, color );
					};

					*dest = color;
				}

				for ( n = rightBgCount ; n > 0; n--, dest++ ) { *dest = bg; }
			}
		}

		return true;
	}


	void FFace::OutTextF( wal::GC& gc, int x, int y, const  unicode_t* text, int count )
	{
		MutexLock lock( &mutex );

		if ( !CheckLib() ) { return; }

		if ( !face ) { return; }

		if ( count < 0 ) { count = unicode_strlen( text ); }

		for ( int i = 0; i < count; i++ )
		{
			OutCharF( gc, &x, &y, text[i] );
		}
	}

	void FFace::OutText( wal::GC& gc, int x, int y, const  unicode_t* text, int count )
	{
		MutexLock lock( &mutex );

		if ( !CheckLib() ) { return; }

		if ( !face ) { return; }

		if ( count < 0 ) { count = unicode_strlen( text ); }

		for ( int i = 0; i < count; i++ )
		{
			OutChar( gc, &x, &y, text[i] );
		}
	}

	inline FT_GlyphSlot GetSlot( FT_Face face, unicode_t* pc )
	{
		if ( !face ) { return 0; }

		FT_UInt  glyph_index = FT_Get_Char_Index( face, *pc );

		if ( !glyph_index )
		{
			glyph_index = FT_Get_Char_Index( face, '.' );

			if ( !glyph_index ) { return 0; }

			*pc = '.';
		}

		if ( FT_Load_Glyph( face, glyph_index, FT_LOAD_DEFAULT /*| FT_LOAD_TARGET_LIGHT*/ ) ) { return 0; }

		if ( FT_Render_Glyph( face->glyph, /*FT_RENDER_MODE_LCD*/ FT_RENDER_MODE_NORMAL ) ) { return 0; }

		return face->glyph;
	}


	cpoint FFace::GetTextExtents( const unicode_t* text, int count )
	{
		MutexLock lock( &mutex );

		if ( !CheckLib() ) { return cpoint( 0, 0 ); }

		if ( !face ) { return cpoint( 0, 0 ); }

		if ( count < 0 ) { count = unicode_strlen( text ); }

		int w = 0;

		for ( int i = 0; i < count; i++, text++ )
		{
			auto iter = ciHash.find( *text );

			CharInfo* pInfo = ( iter == ciHash.end() ) ? nullptr : &( iter->second );

			if ( !pInfo )
			{
				unicode_t c = *text;
				FT_GlyphSlot  slot = GetSlot( face, &c );

				if ( !slot ) { continue; }

				CharInfo info( slot );
				ciHash[c] = info;
				pInfo = &( ciHash[c] );
			}

			w += pInfo->pxWidth;
		};

		return cpoint( w, PxHeight() );
	}

	void FFace::OutCharF( wal::GC& gc, int* px, int* py, unicode_t c )
	{
		int bg = gc.FillRgb();
		int fg = gc.TextRgb();

		auto i = ciHash.find( c );

		CharInfo* pInfo = ( i == ciHash.end() ) ? nullptr : &( i->second );

		if ( pInfo )
		{
			if ( imCache.DrawIfExist( gc, *px, *py , c, bg, fg ) )
			{
				*px += pInfo->pxWidth;
				return;
			}
		};

		FT_GlyphSlot  slot = GetSlot( face, &c );

		if ( !slot ) { return; }

		if ( !pInfo )
		{
			CharInfo info( slot );
			pInfo = &( ciHash[c] = info );
		}

		Image32 im;
		cData.SetBg( bg );
		cData.SetFg( fg );

		if ( !MkImage( im, slot ) )
		{
			gc.FillRect( crect( *px, *py, *px + pInfo->pxWidth, *py + pxHeight ) );
			*px += pInfo->pxWidth;
			return;
		}

		imCache.Draw( gc, *px, *py, c, bg, fg, im );
		*px += pInfo->pxWidth;
	}


	void FFace::OutChar( wal::GC& gc, int* px, int* py, unicode_t c )
	{
		FT_GlyphSlot  slot = GetSlot( face, &c );

		if ( !slot ) { return; }

		int x = *px;
		int y = *py;

		auto i = ciHash.find( c );

		CharInfo* pInfo = ( i == ciHash.end() ) ? nullptr : &( i->second );

		if ( !pInfo )
		{
			CharInfo info( slot );
			pInfo = &( ciHash[c] = info );
		}

		*px += pInfo->pxWidth;

		if ( !slot->bitmap.buffer )
		{
			return;
		}

		int h = slot->bitmap.rows;
		int w = slot->bitmap.width;
		unsigned char* p = slot->bitmap.buffer;

		int fg = gc.TextRgb();

		int left = slot->bitmap_left;
		int top = pxAscender - slot->bitmap_top - 1;

		if ( top < 0 ) { top = 0; }

		for ( int i = 0; i < h; i++ )
			for ( int j = 0; j < w; j++, p++ )
			{
				unsigned c = *p;

				if ( c >= 0x80 )
				{
					gc.SetPixel( x + j + left, y + i + top, fg );
				}
			}
	}


}; //namespace FTU

clPtr<cfont::FTInfo> cfont::GetFTFileInfo( const char* path )
{
	FTU::FFace f;

	if ( f.Load( path, 0, 100 ) ) { return 0; }

	clPtr<cfont::FTInfo> node = new cfont::FTInfo;
	node->name = new_char_str( f.Name() );
	node->styleName = new_char_str( f.StyleName() );

	node->flags = 0;

	if ( f.FaceFlags() & FT_FACE_FLAG_FIXED_WIDTH ) { node->flags |= cfont::FTInfo::FIXED_WIDTH; }


	return node;
}

#endif
