/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#include "ncdialogs.h"
#include "search-dlg.h"
#include "ltext.h"
#include "nceditline.h"

class SearchParamDialog: public NCVertDialog
{
	Layout iL;
public:
	StaticLabel textLabel;
	NCEditLine textEdit;
	SButton  caseButton;

	SearchParamDialog( NCDialogParent* parent, const SearchAndReplaceParams* params );
	virtual bool Command( int id, int subId, Win* win, void* data );
	virtual ~SearchParamDialog();
};

SearchParamDialog::~SearchParamDialog() {}

SearchParamDialog::SearchParamDialog( NCDialogParent* parent, const SearchAndReplaceParams* params )
	:  NCVertDialog( ::createDialogAsChild, 0, parent, utf8_to_unicode( _LT( "Search" ) ).data(), bListOkCancel ),
	   iL( 16, 3 ),
	   textLabel( 0, this, utf8_to_unicode( _LT( "&Search for:" ) ).data(), &textEdit ),
		textEdit( "search-text", 0, this, 0, 50, 7, false, true, false ),
	   caseButton( 0, this, utf8_to_unicode( _LT( "C&ase sensitive" ) ).data(), 0, params->m_CaseSensitive )

{
	if ( params->m_SearchChar )
	{
		const unicode_t Str[] = { params->m_SearchChar, 0 };
		textEdit.SetText( Str, false );
	}
	else
	{
		if ( params->m_SearchText.data() ) { textEdit.SetText( params->m_SearchText.data(), true ); }
	}

	iL.AddWin( &textLabel, 0, 0 );
	textLabel.Enable();
	textLabel.Show();

	iL.AddWin( &textEdit, 0, 1 );
	textEdit.Enable();
	textEdit.Show();
	textEdit.SetFocus();

	iL.AddWin( &caseButton, 1, 1 );
	caseButton.Enable();
	caseButton.Show();

	AddLayout( &iL );
	SetEnterCmd( CMD_OK );

	order.append( &textEdit );
	order.append( &caseButton );

	SetPosition();
}

bool SearchParamDialog::Command( int id, int subId, Win* win, void* data )
{
	return NCVertDialog::Command( id, subId, win, data );
}

bool DoSearchDialog( NCDialogParent* parent, SearchAndReplaceParams* params )
{
	SearchAndReplaceParams LocalParams(*params);

	// always reset
	params->m_SearchChar = 0;

	SearchParamDialog dlg( parent, &LocalParams );

	if ( dlg.DoModal() == CMD_OK )
	{
		params->m_CaseSensitive = dlg.caseButton.IsSet();
		params->m_SearchText = dlg.textEdit.GetText();
		dlg.textEdit.AddCurrentTextToHistory();
		return true;
	}

	return false;
}


/////  SearchFileDialog

class SearchFileParamDialog: public NCVertDialog
{
	Layout iL;
public:
	StaticLabel maskText;
	StaticLabel textText;
	NCEditLine maskEdit;
	NCEditLine textEdit;
	SButton  caseButton;

	SearchFileParamDialog( NCDialogParent* parent, SearchAndReplaceParams* params );
	virtual ~SearchFileParamDialog();
};

SearchFileParamDialog::~SearchFileParamDialog() {}

SearchFileParamDialog::SearchFileParamDialog( NCDialogParent* parent, SearchAndReplaceParams* params )
	:  NCVertDialog( ::createDialogAsChild, 0, parent, utf8_to_unicode( _LT( "Search" ) ).data(), bListOkCancel ),
	   iL( 16, 3 ),
	   maskText( 0, this, utf8_to_unicode( _LT( "File &mask:" ) ).data(), &maskEdit ),
	   textText( 0, this, utf8_to_unicode( _LT( "&Text:" ) ).data(), &textEdit ),
		maskEdit( "fsearch-mask", 0, this, 0, 50, 7, false, true, false ),
		textEdit( "search-text", 0, this, 0, 50, 7, false, true, false ),
	   caseButton( 0, this, utf8_to_unicode( _LT( "C&ase sensitive" ) ).data(), 0, params->m_CaseSensitive )
{
	if ( params->m_SearchMask.data() ) { maskEdit.SetText( params->m_SearchMask.data(), true ); }
	if ( params->m_SearchText.data() ) { textEdit.SetText( params->m_SearchText.data(), true ); }

	iL.AddWin( &maskText, 0, 0 );
	maskText.Enable();
	maskText.Show();
	iL.AddWin( &textText, 1, 0 );
	textText.Enable();
	textText.Show();

	iL.AddWin( &maskEdit, 0, 1 );
	maskEdit.Enable();
	maskEdit.Show();
	maskEdit.SetFocus();

	iL.AddWin( &textEdit, 1, 1 );
	textEdit.Enable();
	textEdit.Show();

	iL.AddWin( &caseButton, 2, 1 );
	caseButton.Enable();
	caseButton.Show();

	AddLayout( &iL );
	SetEnterCmd( CMD_OK );

	order.append( &maskEdit );
	order.append( &textEdit );
	order.append( &caseButton );

	SetPosition();
}

bool DoFileSearchDialog( NCDialogParent* parent, SearchAndReplaceParams* params )
{
	SearchFileParamDialog dlg( parent, params );

	if ( dlg.DoModal() == CMD_OK )
	{
		params->m_CaseSensitive = dlg.caseButton.IsSet();
		
		params->m_SearchMask = dlg.maskEdit.GetText();
		dlg.maskEdit.AddCurrentTextToHistory();
		
		params->m_SearchText = dlg.textEdit.GetText();
		dlg.textEdit.AddCurrentTextToHistory();
		return true;
	}

	return false;
}


//////////////// ReplaceEdit dialog

class ReplaceEditParamDialog: public NCVertDialog
{
	Layout iL;
public:
	StaticLabel fromText;
	StaticLabel toText;
	NCEditLine fromEdit;
	NCEditLine toEdit;
	SButton  caseButton;

	ReplaceEditParamDialog( NCDialogParent* parent, SearchAndReplaceParams* params );
	virtual ~ReplaceEditParamDialog();
};

ReplaceEditParamDialog::~ReplaceEditParamDialog() {}

ReplaceEditParamDialog::ReplaceEditParamDialog( NCDialogParent* parent, SearchAndReplaceParams* params )
	:  NCVertDialog( ::createDialogAsChild, 0, parent, utf8_to_unicode( _LT( "Replace" ) ).data(), bListOkCancel ),
	   iL( 16, 3 ),
	   fromText( 0, this, utf8_to_unicode( _LT( "&Search for:" ) ).data(), &fromEdit ),
	   toText( 0, this, utf8_to_unicode( _LT( "&Replace with:" ) ).data(), &toEdit ),
		fromEdit( "search-text", 0, this, 0, 50, 7, false, true, false ),
		toEdit( "replace-text", 0, this, 0, 50, 7, false, true, false ),
	   caseButton( 0, this, utf8_to_unicode( _LT( "C&ase sensitive" ) ).data(), 0, params->m_CaseSensitive )
{
	if ( params->m_SearchText.data() ) { fromEdit.SetText( params->m_SearchText.data(), true ); }

	if ( params->m_ReplaceTo.data() ) { toEdit.SetText( params->m_ReplaceTo.data(), true ); }

	iL.AddWinAndEnable( &fromText, 0, 0 );
	iL.AddWinAndEnable( &toText,   1, 0 );
	iL.AddWinAndEnable( &fromEdit, 0, 1 );
	fromEdit.SetFocus();
	iL.AddWinAndEnable( &toEdit,   1, 1 );
	iL.AddWinAndEnable( &caseButton, 2, 1 );

	AddLayout( &iL );
	SetEnterCmd( CMD_OK );

	order.append( &fromEdit );
	order.append( &toEdit );
	order.append( &caseButton );

	SetPosition();
}

bool DoReplaceEditDialog( NCDialogParent* parent, SearchAndReplaceParams* params )
{
	ReplaceEditParamDialog dlg( parent, params );

	if ( dlg.DoModal() == CMD_OK )
	{
		params->m_CaseSensitive = dlg.caseButton.IsSet();
		
		params->m_SearchText = dlg.fromEdit.GetText();
		dlg.fromEdit.AddCurrentTextToHistory();
		
		params->m_ReplaceTo = dlg.toEdit.GetText();
		dlg.toEdit.AddCurrentTextToHistory();
		return true;
	}

	return false;
}
