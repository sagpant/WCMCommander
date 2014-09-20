/*
   Copyright (c) by Valery Goryachev (Wal)
*/

#pragma once

namespace wal
{

	class IntList
	{
		struct Node
		{
			int data;
			Node( int d ): data( d ) {}
			Node( const Node& a ): data( a.data ) {}
			Node& operator=(const Node& a) = delete;
			const int& key() const { return data; }
			unsigned intKey() const { return ( unsigned )data; }
			Node* next;
		private:
			Node() {}
		};
		internal_hash<Node, int, false> hash;
	public:
		IntList() {}
		IntList( const IntList& a ): hash( a.hash ) {}
		int count() const { return hash.count(); };
		bool exist( int n ) {   return hash.find( unsigned( n ), n ) != 0;   }
		bool operator []( int n ) { return exist( n ); }
		IntList& operator = ( const IntList& a ) {   if ( this != &a ) { hash = a.hash; }   return *this;}
		void put( int n )
		{
			Node* p = hash.find( unsigned( n ), n );

			if ( !p )
			{
				hash.append( unsigned( n ), new Node( n ) );
			}
		}
		bool del( int n )   {return hash.del( unsigned( n ), n, false );}
		void foreach( void ( *f )( int  key, void* ), void* parm )
		{
			for ( hash_iterator<Node> i = hash.first(); i.valid(); i.next() )
			{
				f( i.get()->data, parm );
			}
		}
		std::vector<int> keys()
		{
			int n = hash.count();
			std::vector<int> ret( n );
			int j = 0;

			for ( hash_iterator<Node> i = hash.first(); i.valid(); i.next(), j++ )
			{
				ret[j] = i.get()->intKey();
			}

			return ret;
		}
		void clear() { hash.clear();}
		~IntList() {}
	};

	template <class T, unsigned BS = 1024, unsigned STEP = 256> class cfbarray
	{
		int dCount;
		ccollect< std::vector<T>, STEP > data;
	public:
		cfbarray(): dCount( 0 ) {};
		cfbarray( unsigned n ): dCount( n ), data( ( n + BS - 1 ) / BS ) { for ( int i = 0; i < data.count(); i++ ) { data[i] = std::vector<T>( BS ); } }
		void alloc( unsigned n )
		{
			unsigned b = ( n + BS - 1 ) / BS;

			if ( unsigned( data.count() ) > b ) { data.del( b, unsigned( data.count() ) - b ); }

			while ( unsigned( data.count() ) < b ) { data.append( std::vector<T>( BS ) ); }

			dCount = n;
		};
		int count() const { return dCount; }
		void clear() { data.clear(); dCount = 0; }
		void append( int n = 1 ) { alloc( dCount + n ); }
		void append( T& a ) { append(); get( dCount - 1 ) = a; }
		T& get( int n ) { ASSERT( n >= 0 && n < dCount ); return data[n / BS][n % BS]; };
		T& operator []( int n ) { return get( n ); }

		void Sort( bool lessf( T* a, T* b, void* d ), void* d )
		{
			if ( dCount <= 1 ) { return; }

			int i, cnt = dCount;
			std::vector<T> list( cnt );

			for ( i = 0; i < cnt; i++ ) { list[i] = get( i ); }

			sort2m_data<T, void>( list.ptr(), cnt, lessf, d );

			for ( i = 0; i < cnt; i++ ) { get( i ) = list[i]; }
		}

		~cfbarray() {};
	};


}; //namespace wal
