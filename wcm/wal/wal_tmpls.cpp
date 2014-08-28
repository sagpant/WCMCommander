/*
   Copyright (c) by Valery Goryachev (Wal)
*/


#include "wal_tmpls.h"

namespace wal
{

	static HashIndex hashPrimes[] =
	{
		7,
		17,
		37,
		79,
		163,
		331,
		673,
		1361,
		2729,
		5471,
		10949,
		21911,
		43853,
		87719,
		175447,
		350899,
		701819,
		1403641,
		2807303,
		5614657,
		11229331,
		22458671,
		44917381,
		89834777,
		179669557,
		359339171,
		718678369,
		1437356741
	};


	HashIndex hash_upper_table_size( HashIndex size )
	{
		const int count = sizeof( hashPrimes ) / sizeof( HashIndex );
		int L = 0, R = count;

		while ( L < R )
		{
			unsigned i = ( L + R ) / 2;

			if ( size < hashPrimes[i] )
			{
				R = i;
			}
			else
			{
				L = i + 1;
			}
		}

		return hashPrimes[R < count ? R : R - 1];
	}

	HashIndex hash_lover_table_size( HashIndex size )
	{
		const int count = sizeof( hashPrimes ) / sizeof( HashIndex );
		int L = 0, R = count;

		while ( L < R )
		{
			unsigned i = ( L + R ) / 2;

			if ( size <= hashPrimes[i] )
			{
				R = i;
			}
			else
			{
				L = i + 1;
			}
		}

		return hashPrimes[R > 0 ? R - 1 : 0];
	}

}
/*

in MS Visual C++
disable  precompiled headers for this file

*/
