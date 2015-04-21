/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * wcm@linderdaum.com
 */

#include "smblogon.h"

#include "ncdialogs.h"
#include "string-util.h"
#include "ltext.h"
#include "nceditline.h"

#ifdef LIBSMBCLIENT_EXIST

class SmbLogonDialog: public NCVertDialog
{
	Layout iL;
public:
	StaticLine text;

	bool serverInput;

	StaticLine serverText;
	StaticLine domainText;
	StaticLine userText;
	StaticLine passwordText;

	clNCEditLine serverEdit;
	clNCEditLine domainEdit;
	clNCEditLine userEdit;
	EditLine passwordEdit;

	SmbLogonDialog( NCDialogParent* parent, FSSmbParam& params, bool enterServer );
	virtual ~SmbLogonDialog();
};

SmbLogonDialog::~SmbLogonDialog() {}

SmbLogonDialog::SmbLogonDialog( NCDialogParent* parent, FSSmbParam& params, bool enterServer )
	:  NCVertDialog( ::createDialogAsChild, 0, parent, utf8_to_unicode( _LT( "SMB logon" ) ).data(), bListOkCancel ),
	   iL( 16, 2 ),
	   text( 0, this, utf8_to_unicode( carray_cat<char>( _LT( "&Server:" ), const_cast<char*>( params.server ) ).data() ).data() ),
	   serverInput( enterServer ),
	   serverText( 0, this, utf8_to_unicode( _LT( "&Server:" ) ).data() ),
	   domainText( 0, this, utf8_to_unicode( _LT( "&Domain:" ) ).data() ),
	   userText( 0, this, utf8_to_unicode( _LT( "&Login:" ) ).data() ),
	   passwordText( 0, this, utf8_to_unicode( _LT( "Pass&word:" ) ).data() ),
		serverEdit( EDIT_FIELD_SMB_SERVER, 0, this, 0, 16, 7 ),
		domainEdit( EDIT_FIELD_SMB_DOMAIN, 0, this, 0, 16, 7 ),
		userEdit( EDIT_FIELD_SMB_USER, 0, this, 0, 16, 7 ),
	   passwordEdit( 0, this, 0, 0, 16 )
{

	serverEdit.SetText( utf8_to_unicode( const_cast<char*>( params.server ) ).data(), true );
	domainEdit.SetText( utf8_to_unicode( const_cast<char*>( params.domain ) ).data(), true );
	userEdit.SetText( utf8_to_unicode( const_cast<char*>( params.user ) ).data(), true );
	passwordEdit.SetText( utf8_to_unicode( const_cast<char*>( params.pass ) ).data(), true );

	bool focus = false;

	if ( serverInput )
	{
		iL.AddWin( &serverText,  0, 0 );
		serverText.Enable();
		serverText.Show();
		iL.AddWin( &serverEdit,  0, 1 );
		serverEdit.Enable();
		serverEdit.Show();

		if ( !focus && !params.server[0] ) { serverEdit.SetFocus(); focus = true; }
	}
	else
	{
		iL.AddWin( &text, 0, 0, 0, 1 );
		text.Enable();
		text.Show();
	}

	iL.AddWin( &domainText,  1, 0 );
	domainText.Enable();
	domainText.Show();
	iL.AddWin( &domainEdit,  1, 1 );
	domainEdit.Enable();
	domainEdit.Show();

	if ( !focus && !params.domain[0] ) { domainEdit.SetFocus(); focus = true; }

	iL.AddWin( &userText, 2, 0 );
	userText.Enable();
	userText.Show();
	iL.AddWin( &userEdit, 2, 1 );
	userEdit.Enable();
	userEdit.Show();

	if ( !focus && !params.user[0] ) { userEdit.SetFocus(); focus = true; }

	passwordEdit.SetPasswordMode();
	iL.AddWin( &passwordText, 3, 0 );
	passwordText.Enable();
	passwordText.Show();
	iL.AddWin( &passwordEdit, 3, 1 );
	passwordEdit.Enable();
	passwordEdit.Show();

	if ( !focus && !params.pass[0] ) { passwordEdit.SetFocus(); focus = true; }

	AddLayout( &iL );
	SetEnterCmd( CMD_OK );

	order.append( &serverEdit );
	order.append( &domainEdit );
	order.append( &userEdit );
	order.append( &passwordEdit );

	SetPosition();
}

static void Copy( volatile char* dest, const char* src, int destSize )
{
	int n = destSize;

	for ( ; *src && n > 1; src++, dest++, n-- ) { *dest = *src; }

	*dest = 0;
}

bool GetSmbLogon( NCDialogParent* parent, FSSmbParam& params, bool enterServer )
{
	SmbLogonDialog dlg( parent, params, enterServer );

	if ( dlg.DoModal() == CMD_OK )
	{
		if ( enterServer )
		{
			Copy( params.server, dlg.serverEdit.GetTextStr().data() , sizeof( params.server ) );
			dlg.serverEdit.AddCurrentTextToHistory();
		}

		Copy( params.domain, dlg.domainEdit.GetTextStr().data() , sizeof( params.domain ) );
		dlg.domainEdit.AddCurrentTextToHistory();
		
		Copy( params.user, dlg.userEdit.GetTextStr().data() , sizeof( params.user ) );
		dlg.userEdit.AddCurrentTextToHistory();
		
		Copy( params.pass, dlg.passwordEdit.GetTextStr().data() , sizeof( params.pass ) );
		params.isSet = true;
		return true;
	}

	return false;
}

#endif
