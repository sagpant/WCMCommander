#include "ncdialogs.h"
#include "search-dlg.h"
#include "ltext.h"

class SearchParamDialog: public NCVertDialog {
	Layout iL;
public:	
	EditLine textEdit;
	SButton  caseButton;

	SearchParamDialog(NCDialogParent *parent, SearchAndReplaceParams *params);
	virtual bool Command(int id, int subId, Win *win, void *data);
	virtual ~SearchParamDialog();
};

SearchParamDialog::~SearchParamDialog(){}

SearchParamDialog::SearchParamDialog(NCDialogParent *parent, SearchAndReplaceParams *params)
:	NCVertDialog(::createDialogAsChild, 0, parent, utf8_to_unicode(_LT("Search")).ptr(), bListOkCancel),
	iL(16, 3),
	textEdit	(0, this, 0, 0, 50),
	caseButton(0, this, utf8_to_unicode(_LT("Case sensitive")).ptr(), 0, params->sens)
{
	if (params->txt.ptr()) textEdit.SetText(params->txt.ptr(), true);
	
	iL.AddWin(&textEdit,	0, 0); textEdit.Enable(); textEdit.Show(); 
	textEdit.SetFocus();
	
	iL.AddWin(&caseButton, 1, 0); caseButton.Enable(); caseButton.Show(); 
	
	AddLayout(&iL);
	SetEnterCmd(CMD_OK);
	
	order.append(&textEdit);
	order.append(&caseButton);
	
	SetPosition();
}

bool SearchParamDialog::Command(int id, int subId, Win *win, void *data)
{
	return NCVertDialog::Command(id, subId, win, data);
}

bool DoSearchDialog(NCDialogParent *parent, SearchAndReplaceParams *params)
{
	SearchParamDialog dlg(parent, params);
	if (dlg.DoModal() == CMD_OK)
	{
		params->sens = dlg.caseButton.IsSet();
		params->txt = new_unicode_str(dlg.textEdit.GetText().ptr());
		return true;
	}
	return false;
}


/////  SearchFileDialog

class SearchFileParamDialog: public NCVertDialog {
	Layout iL;
public:	
	StaticLine maskText;
	StaticLine textText;
	EditLine maskEdit;
	EditLine textEdit;
	SButton  caseButton;

	SearchFileParamDialog(NCDialogParent *parent, SearchAndReplaceParams *params);
	virtual ~SearchFileParamDialog();
};

SearchFileParamDialog::~SearchFileParamDialog(){}

SearchFileParamDialog::SearchFileParamDialog(NCDialogParent *parent, SearchAndReplaceParams *params)
:	NCVertDialog(::createDialogAsChild, 0, parent, utf8_to_unicode(_LT("Search")).ptr(), bListOkCancel),
	iL(16, 3),
	maskText(0, this, utf8_to_unicode(_LT("File mask:")).ptr()),
	textText(0, this, utf8_to_unicode(_LT("Text:")).ptr()),
	maskEdit	(0, this, 0, 0, 50),
	textEdit	(0, this, 0, 0, 50),
	caseButton(0, this, utf8_to_unicode(_LT("Case sensitive")).ptr(), 0, params->sens)
{
	if (params->mask.ptr()) maskEdit.SetText(params->mask.ptr(), true);
	if (params->txt.ptr()) textEdit.SetText(params->txt.ptr(), true);
	
	iL.AddWin(&maskText,	0, 0); maskText.Enable(); maskText.Show();
	iL.AddWin(&textText,	1, 0); textText.Enable(); textText.Show();
	
	iL.AddWin(&maskEdit,	0, 1); maskEdit.Enable(); maskEdit.Show(); 
	maskEdit.SetFocus();
	
	iL.AddWin(&textEdit,	1, 1); textEdit.Enable(); textEdit.Show(); 
	
	iL.AddWin(&caseButton, 2, 1); caseButton.Enable(); caseButton.Show(); 
	
	AddLayout(&iL);
	SetEnterCmd(CMD_OK);
	
	order.append(&maskEdit);
	order.append(&textEdit);
	order.append(&caseButton);
	
	SetPosition();
}

bool DoFileSearchDialog(NCDialogParent *parent, SearchAndReplaceParams *params)
{
	SearchFileParamDialog dlg(parent, params);
	if (dlg.DoModal() == CMD_OK)
	{
		params->sens = dlg.caseButton.IsSet();
		params->mask = new_unicode_str(dlg.maskEdit.GetText().ptr());
		params->txt = new_unicode_str(dlg.textEdit.GetText().ptr());
		return true;
	}
	return false;
}


//////////////// ReplaceEdit dialog

class ReplaceEditParamDialog: public NCVertDialog {
	Layout iL;
public:	
	StaticLine fromText;
	StaticLine toText;
	EditLine fromEdit;
	EditLine toEdit;
	SButton  caseButton;

	ReplaceEditParamDialog(NCDialogParent *parent, SearchAndReplaceParams *params);
	virtual ~ReplaceEditParamDialog();
};

ReplaceEditParamDialog::~ReplaceEditParamDialog(){}

ReplaceEditParamDialog::ReplaceEditParamDialog(NCDialogParent *parent, SearchAndReplaceParams *params)
:	NCVertDialog(::createDialogAsChild, 0, parent, utf8_to_unicode(_LT("Replace")).ptr(), bListOkCancel),
	iL(16, 3),
	fromText(0, this, utf8_to_unicode(_LT("Search for:")).ptr()),
	toText(0, this, utf8_to_unicode(_LT("Replace with:")).ptr()),
	fromEdit	(0, this, 0, 0, 50),
	toEdit	(0, this, 0, 0, 50),
	caseButton(0, this, utf8_to_unicode(_LT("Case sensitive")).ptr(), 0, params->sens)
{
	if (params->txt.ptr()) fromEdit.SetText(params->txt.ptr(), true);
	if (params->to.ptr()) toEdit.SetText(params->to.ptr(), true);
	
	iL.AddWin(&fromText,	0, 0); fromText.Enable(); fromText.Show();
	iL.AddWin(&toText,	1, 0); toText.Enable(); toText.Show();
	
	iL.AddWin(&fromEdit,	0, 1); fromEdit.Enable(); fromEdit.Show(); 
	fromEdit.SetFocus();
	
	iL.AddWin(&toEdit,	1, 1); toEdit.Enable(); toEdit.Show(); 
	
	iL.AddWin(&caseButton, 2, 1); caseButton.Enable(); caseButton.Show(); 
	
	AddLayout(&iL);
	SetEnterCmd(CMD_OK);
	
	order.append(&fromEdit);
	order.append(&toEdit);
	order.append(&caseButton);
	
	SetPosition();
}

bool DoReplaceEditDialog(NCDialogParent *parent, SearchAndReplaceParams *params)
{
	ReplaceEditParamDialog dlg(parent, params);
	if (dlg.DoModal() == CMD_OK)
	{
		params->sens = dlg.caseButton.IsSet();
		params->txt = new_unicode_str(dlg.fromEdit.GetText().ptr());
		params->to = new_unicode_str(dlg.toEdit.GetText().ptr());
		return true;
	}
	return false;
}



