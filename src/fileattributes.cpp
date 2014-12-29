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
#include "panel.h"

class clFileAttributesWin: public NCVertDialog
{
public:
	clFileAttributesWin( NCDialogParent* parent, PanelWin* Panel )
	 : NCVertDialog( ::createDialogAsChild, 0, parent, utf8_to_unicode(_LT("Attributes")).data(), bListOkCancel )
	 , m_Panel( Panel )
	 , m_Node( Panel ? Panel->GetCurrent() : nullptr )
	 , m_Layout( 17, 2 )
	 , m_CaptionText( 0, this, utf8_to_unicode( _LT( "" ) ).data(), nullptr, StaticLine::CENTER )
	 , m_FileNameText( 0, this, utf8_to_unicode( _LT( "" ) ).data(), nullptr, StaticLine::CENTER )
	{
		if ( m_Panel )
		{
			const unicode_t* FileName = m_Panel->GetCurrentFileName();

			m_FileNameText.SetText( FileName );
		}

		if ( m_Node && m_Node->IsDir() )
		{
			m_CaptionText.SetText( utf8_to_unicode( _LT("Change attributes for folder:") ).data() );
		}
		else
		{
			m_CaptionText.SetText( utf8_to_unicode( _LT("Change file attributes for:") ).data() );
		}

		m_Layout.AddWinAndEnable( &m_CaptionText, 0, 0 );
		m_Layout.AddWinAndEnable( &m_FileNameText, 1, 0 );
		
		AddLayout( &m_Layout );

		SetPosition();
	}
private:
	PanelWin* m_Panel;
	FSNode* m_Node;

	Layout m_Layout;

	StaticLine m_CaptionText;
	StaticLine m_FileNameText;
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

