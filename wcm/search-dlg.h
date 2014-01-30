#ifndef SEARCH_DLG_H
#define SEARCH_DLG_H



struct SearchParams {
	carray<unicode_t> txt;
	bool sens;
	SearchParams():sens(false){}
	SearchParams(const SearchParams &a) { txt = new_unicode_str(a.txt.const_ptr()); }
	SearchParams& operator = (const SearchParams &a){  txt = new_unicode_str(a.txt.const_ptr()); return *this; }
};

bool DoSearchDialog(NCDialogParent *parent, SearchParams *params);


struct SearchFileParams {
	carray<unicode_t> mask;
	carray<unicode_t> txt;
	bool sens;
	SearchFileParams():sens(false){}
	SearchFileParams(const SearchFileParams &a) { mask = new_unicode_str(a.mask.const_ptr()); txt = new_unicode_str(a.txt.const_ptr()); }
	SearchFileParams& operator = (const SearchFileParams &a){  mask = new_unicode_str(a.mask.const_ptr()); txt = new_unicode_str(a.txt.const_ptr()); return *this; }
};

bool DoFileSearchDialog(NCDialogParent *parent, SearchFileParams *params);

struct ReplaceEditParams {
	carray<unicode_t> from;
	carray<unicode_t> to;
	bool sens;
	ReplaceEditParams():sens(false){}
	ReplaceEditParams(const ReplaceEditParams &a) { from = new_unicode_str(a.from.const_ptr()); to = new_unicode_str(a.to.const_ptr()); }
	ReplaceEditParams& operator = (const ReplaceEditParams &a){  from = new_unicode_str(a.from.const_ptr()); to = new_unicode_str(a.to.const_ptr()); return *this; }
};

bool DoReplaceEditDialog(NCDialogParent *parent, ReplaceEditParams *params);


#endif
