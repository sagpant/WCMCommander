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
			Node& operator=( const Node& a ) = delete;
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

}; //namespace wal
