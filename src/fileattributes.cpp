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

/*
Windows:
	Read only
	Archive
	Hidden
	System
	Compressed
	Encrypted

	Not Indexed
	Sparse
	Temporary
	Offline
	Reparse point
	Virtual

*NIX:
	Read by owner
	Write by owner
	Execute by owner
	Read by group
	Write by group
	Execute by group
	Read by others
	Write by others
	Execute by others
	Owner name
	Group Name

Common:
	Last write time
	Creation time
	Last access time
	Change time
*/

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
#if defined(_WIN32)
	 , m_ReadOnly( 0, this, utf8_to_unicode( _LT( "Read only" ) ).data(), 0, false )
	 , m_Archive( 0, this, utf8_to_unicode( _LT( "Archive" ) ).data(), 0, false )
	 , m_Hidden( 0, this, utf8_to_unicode( _LT( "Hidden" ) ).data(), 0, false )
	 , m_System( 0, this, utf8_to_unicode( _LT( "System" ) ).data(), 0, false )
	 , m_Compressed( 0, this, utf8_to_unicode( _LT( "Compressed" ) ).data(), 0, false )
	 , m_Encrypted( 0, this, utf8_to_unicode( _LT( "Encrypted" ) ).data(), 0, false )
	 , m_NotIndexed( 0, this, utf8_to_unicode( _LT( "NotIndexed" ) ).data(), 0, false )
	 , m_Sparse( 0, this, utf8_to_unicode( _LT( "Sparse" ) ).data(), 0, false )
	 , m_Temporary( 0, this, utf8_to_unicode( _LT( "Temporary" ) ).data(), 0, false )
	 , m_Offline( 0, this, utf8_to_unicode( _LT( "Offline" ) ).data(), 0, false )
	 , m_ReparsePoint( 0, this, utf8_to_unicode( _LT( "ReparsePoint" ) ).data(), 0, false )
	 , m_Virtual( 0, this, utf8_to_unicode( _LT( "Virtual" ) ).data(), 0, false )
#else
#endif
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
#if defined(_WIN32)
		m_Layout.AddWinAndEnable( &m_ReadOnly, 2, 0 );
		m_Layout.AddWinAndEnable( &m_Archive, 3, 0 );
		m_Layout.AddWinAndEnable( &m_Hidden, 4, 0 );
		m_Layout.AddWinAndEnable( &m_System, 5, 0 );
		m_Layout.AddWinAndEnable( &m_Compressed, 6, 0 );
		m_Layout.AddWinAndEnable( &m_Encrypted, 7, 0 );
		m_Layout.AddWinAndEnable( &m_NotIndexed, 2, 1 );
		m_Layout.AddWinAndEnable( &m_Sparse, 3, 1 );
		m_Layout.AddWinAndEnable( &m_Temporary, 4, 1 );
		m_Layout.AddWinAndEnable( &m_Offline, 5, 1 );
		m_Layout.AddWinAndEnable( &m_ReparsePoint, 6, 1 );
		m_Layout.AddWinAndEnable( &m_Virtual, 7, 1 );
		// disable editing of these properties
		m_Compressed.Enable( false );
		m_Encrypted.Enable( false );
		m_NotIndexed.Enable( false );
		m_Sparse.Enable( false );
		m_Temporary.Enable( false );
		m_Offline.Enable( false );
		m_ReparsePoint.Enable( false );
		m_Virtual.Enable( false );
#else
#endif
		
		AddLayout( &m_Layout );

		SetPosition();
	}
private:
	PanelWin* m_Panel;
	FSNode* m_Node;

	Layout m_Layout;

	StaticLine m_CaptionText;
	StaticLine m_FileNameText;
#if defined(_WIN32)
	SButton m_ReadOnly;
	SButton m_Archive;
	SButton m_Hidden;
	SButton m_System;
	SButton m_Compressed;
	SButton m_Encrypted;
	SButton m_NotIndexed;
	SButton m_Sparse;
	SButton m_Temporary;
	SButton m_Offline;
	SButton m_ReparsePoint;
	SButton m_Virtual;
#else
#endif
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

