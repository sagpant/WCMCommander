#ifndef SEARCH_DLG_H
#define SEARCH_DLG_H

struct SearchAndReplaceParams
{
	carray<unicode_t> mask;
	carray<unicode_t> txt;
	carray<unicode_t> to;
	bool sens;
	SearchAndReplaceParams(): sens( false ) {}
	SearchAndReplaceParams( const SearchAndReplaceParams& a )
	{
		mask = new_unicode_str( a.mask.data() );
		txt = new_unicode_str( a.txt.data() );
		to = new_unicode_str( a.to.data() );
	}
	SearchAndReplaceParams& operator = ( const SearchAndReplaceParams& a )
	{
		mask = new_unicode_str( a.mask.data() );
		txt = new_unicode_str( a.txt.data() );
		to = new_unicode_str( a.to.data() );
		return *this;
	}
};

bool DoSearchDialog( NCDialogParent* parent, SearchAndReplaceParams* params );
bool DoFileSearchDialog( NCDialogParent* parent, SearchAndReplaceParams* params );
bool DoReplaceEditDialog( NCDialogParent* parent, SearchAndReplaceParams* params );

#endif
