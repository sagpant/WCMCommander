/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#include "fileattributes.h"

#include "ncdialogs.h"
#include "dialog_enums.h"
#include "dialog_helpers.h"
#include "ltext.h"

class clFileAttributesWin: public NCVertDialog
{
public:
	clFileAttributesWin( NCDialogParent* parent, PanelWin* Panel )
	 : NCVertDialog( ::createDialogAsChild, 0, parent, utf8_to_unicode(_LT("Attributes")).data(), bListOkCancel )
	 , m_Panel( Panel )
	 , m_Layout( 17, 2 )
	 , m_CaptionText( 0, this, utf8_to_unicode( _LT( "Change file attributes for" ) ).data(), nullptr )
	 , m_FileNameText( 0, this, utf8_to_unicode( _LT( "" ) ).data(), nullptr )
	{
		m_Layout.AddWinAndEnable( &m_CaptionText, 0, 0 );
		m_Layout.AddWinAndEnable( &m_FileNameText, 1, 0 );
		
		AddLayout( &m_Layout );

		SetPosition();
	}
private:
	PanelWin* m_Panel;

	Layout m_Layout;

	StaticLabel m_CaptionText;
	StaticLabel m_FileNameText;
};

bool FileAttributesDlg( NCDialogParent* Parent, PanelWin* Panel )
{
	clFileAttributesWin Dialog( Parent, Panel );
	Dialog.SetEnterCmd( 0 );

	if ( Dialog.DoModal( ) == CMD_OK )
	{
		// TODO: apply changes
		return true;
	}

	return false;
}

