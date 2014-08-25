#ifndef SEARCH_DLG_H
#define SEARCH_DLG_H

struct SearchAndReplaceParams {
	carray<unicode_t> mask;
	carray<unicode_t> txt;
	carray<unicode_t> to;
	bool sens;
	SearchAndReplaceParams():sens(false){}
	SearchAndReplaceParams(const SearchAndReplaceParams& a)
	{
		mask = new_unicode_str(a.mask.const_ptr());
		txt = new_unicode_str(a.txt.const_ptr());
		to = new_unicode_str(a.to.const_ptr());
	}
	SearchAndReplaceParams& operator = (const SearchAndReplaceParams& a)
	{
		mask = new_unicode_str(a.mask.const_ptr());
		txt = new_unicode_str(a.txt.const_ptr());
		to = new_unicode_str(a.to.const_ptr());
		return *this;
	}
};

bool DoSearchDialog(NCDialogParent *parent, SearchAndReplaceParams *params);
bool DoFileSearchDialog(NCDialogParent *parent, SearchAndReplaceParams *params);
bool DoReplaceEditDialog(NCDialogParent *parent, SearchAndReplaceParams *params);

#endif
