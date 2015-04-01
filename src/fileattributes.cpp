/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * walcommander@linderdaum.com
 */

#include "fileattributes.h"

#include "ncdialogs.h"
#include "dialog_enums.h"
#include "dialog_helpers.h"
#include "ltext.h"
#include "panel.h"
#include "vfs.h"

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
	Change time (unsupported yet)
*/

class clFileAttributesWin: public NCVertDialog
{
public:
	clFileAttributesWin( NCDialogParent* parent, PanelWin* Panel )
	 : NCVertDialog( ::createDialogAsChild, 0, parent, utf8_to_unicode(_LT("Attributes")).data(), bListOkCancel )
	 , m_Panel( Panel )
	 , m_Node( Panel ? Panel->GetCurrent() : nullptr )
	 , m_URI( Panel ? Panel->UriOfCurrent() : FSString() )
	 , m_Layout( 17, 3 )
	 , m_CaptionText( 0, this, utf8_to_unicode( _LT( "" ) ).data(), nullptr, StaticLine::CENTER )
	 , m_FileNameText( uiValue, this, utf8_to_unicode( _LT( "" ) ).data(), nullptr, StaticLine::CENTER )
#if defined(_WIN32)
	 , m_ReadOnly( uiVariable, this, utf8_to_unicode( _LT( "Read only" ) ).data(), 0, false )
	 , m_Archive( uiVariable, this, utf8_to_unicode( _LT( "Archive" ) ).data(), 0, false )
	 , m_Hidden( uiVariable, this, utf8_to_unicode( _LT( "Hidden" ) ).data(), 0, false )
	 , m_System( uiVariable, this, utf8_to_unicode( _LT( "System" ) ).data(), 0, false )
	 , m_Compressed( 0, this, utf8_to_unicode( _LT( "Compressed" ) ).data(), 0, false )
	 , m_Encrypted( 0, this, utf8_to_unicode( _LT( "Encrypted" ) ).data(), 0, false )
	 , m_NotIndexed( 0, this, utf8_to_unicode( _LT( "NotIndexed" ) ).data(), 0, false )
	 , m_Sparse( 0, this, utf8_to_unicode( _LT( "Sparse" ) ).data(), 0, false )
	 , m_Temporary( 0, this, utf8_to_unicode( _LT( "Temporary" ) ).data(), 0, false )
	 , m_Offline( 0, this, utf8_to_unicode( _LT( "Offline" ) ).data(), 0, false )
	 , m_ReparsePoint( 0, this, utf8_to_unicode( _LT( "ReparsePoint" ) ).data(), 0, false )
	 , m_Virtual( 0, this, utf8_to_unicode( _LT( "Virtual" ) ).data(), 0, false )
#else
	 , m_UserRead( uiVariable, this, utf8_to_unicode( _LT( "User &read" ) ).data(), 0, false )
	 , m_UserWrite( uiVariable, this, utf8_to_unicode( _LT( "User &write" ) ).data(), 0, false )
    , m_UserExecute( uiVariable, this, utf8_to_unicode( _LT( "User e&xecute" ) ).data(), 0, false )
	 , m_GroupRead( uiVariable, this, utf8_to_unicode( _LT( "&Group read" ) ).data(), 0, false )
	 , m_GroupWrite( uiVariable, this, utf8_to_unicode( _LT( "Group write" ) ).data(), 0, false )
	 , m_GroupExecute( uiVariable, this, utf8_to_unicode( _LT( "Group execute" ) ).data(), 0, false )
	 , m_OthersRead( uiVariable, this, utf8_to_unicode( _LT( "&Others read" ) ).data(), 0, false )
	 , m_OthersWrite( uiVariable, this, utf8_to_unicode( _LT( "Others write" ) ).data(), 0, false )
	 , m_OthersExecute( uiVariable, this, utf8_to_unicode( _LT( "Others execute" ) ).data(), 0, false )
#endif
	// time
	 , m_LastWriteTimeLabel( 0, this, utf8_to_unicode( _LT( "Last write time" ) ).data(), nullptr, StaticLine::LEFT )
	 , m_CreationTimeLabel( 0, this, utf8_to_unicode( _LT( "Creation time" ) ).data(), nullptr, StaticLine::LEFT )
	 , m_LastAccessTimeLabel( 0, this, utf8_to_unicode( _LT( "Last access time" ) ).data(), nullptr, StaticLine::LEFT )
	 , m_ChangeTimeLabel( 0, this, utf8_to_unicode( _LT( "Change time" ) ).data(), nullptr, StaticLine::LEFT )
	 //
	 , m_LastWriteDate( 0, this, nullptr, utf8_to_unicode( _LT( "DD.MM.YYYY" ) ).data(), 9 )
	 , m_CreationDate( 0, this, nullptr, utf8_to_unicode( _LT( "DD.MM.YYYY" ) ).data(), 9 )
	 , m_LastAccessDate( 0, this, nullptr, utf8_to_unicode( _LT( "DD.MM.YYYY" ) ).data(), 9 )
	 , m_ChangeDate( 0, this, nullptr, utf8_to_unicode( _LT( "DD.MM.YYYY" ) ).data(), 9 )
	 //
	 , m_LastWriteTime( 0, this, nullptr, utf8_to_unicode( _LT( "HH:MM:SS" ) ).data(), 13 )
	 , m_CreationTime( 0, this, nullptr, utf8_to_unicode( _LT( "HH:MM:SS" ) ).data(), 13 )
	 , m_LastAccessTime( 0, this, nullptr, utf8_to_unicode( _LT( "HH:MM:SS" ) ).data(), 13 )
	 , m_ChangeTime( 0, this, nullptr, utf8_to_unicode( _LT( "HH:MM:SS" ) ).data(), 13 )
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

		int Row = 0;

		m_Layout.AddWinAndEnable( &m_CaptionText, Row++, 0 );
		m_Layout.AddWinAndEnable( &m_FileNameText, Row++, 0 );

		int TRow = Row;

#if defined(_WIN32)
		m_Layout.AddWinAndEnable( &m_ReadOnly, Row++, 0 );
		m_Layout.AddWinAndEnable( &m_Archive, Row++, 0 );
		m_Layout.AddWinAndEnable( &m_Hidden, Row++, 0 );
		m_Layout.AddWinAndEnable( &m_System, Row++, 0 );
		m_Layout.AddWinAndEnable( &m_Compressed, Row++, 0 );
		m_Layout.AddWinAndEnable( &m_Encrypted, Row++, 0 );
		Row = TRow;
		m_Layout.AddWinAndEnable( &m_NotIndexed, Row++, 1 );
		m_Layout.AddWinAndEnable( &m_Sparse, Row++, 1 );
		m_Layout.AddWinAndEnable( &m_Temporary, Row++, 1 );
		m_Layout.AddWinAndEnable( &m_Offline, Row++, 1 );
		m_Layout.AddWinAndEnable( &m_ReparsePoint, Row++, 1 );
		m_Layout.AddWinAndEnable( &m_Virtual, Row++, 1 );

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
		m_Layout.AddWinAndEnable( &m_UserRead, Row++, 0 );
		m_Layout.AddWinAndEnable( &m_UserWrite, Row++, 0 );
		m_Layout.AddWinAndEnable( &m_UserExecute, Row++, 0 );
		m_Layout.AddWinAndEnable( &m_GroupRead, Row++, 0 );
		m_Layout.AddWinAndEnable( &m_GroupWrite, Row++, 0 );
		m_Layout.AddWinAndEnable( &m_GroupExecute, Row++, 0 );
		m_Layout.AddWinAndEnable( &m_OthersRead, Row++, 0 );
		m_Layout.AddWinAndEnable( &m_OthersWrite, Row++, 0 );
		m_Layout.AddWinAndEnable( &m_OthersExecute, Row++, 0 );
#endif

		m_Layout.LineSet( Row++, 5 );

		TRow = Row;
		m_Layout.AddWinAndEnable( &m_LastWriteTimeLabel, Row++, 0 );
		m_Layout.AddWinAndEnable( &m_CreationTimeLabel, Row++, 0 );
		m_Layout.AddWinAndEnable( &m_LastAccessTimeLabel, Row++, 0 );
#if defined(HAS_CHANGETIME)
		m_Layout.AddWinAndEnable( &m_ChangeTimeLabel, Row++, 0 );
#endif
		Row = TRow;
		m_Layout.AddWinAndEnable( &m_LastWriteDate, Row++, 1 );
		m_Layout.AddWinAndEnable( &m_CreationDate, Row++, 1 );
		m_Layout.AddWinAndEnable( &m_LastAccessDate, Row++, 1 );
#if defined(HAS_CHANGETIME)
		m_Layout.AddWinAndEnable( &m_ChangeDate, Row++, 1 );
#endif
		Row = TRow;
		m_Layout.AddWinAndEnable( &m_LastWriteTime, Row++, 2 );
		m_Layout.AddWinAndEnable( &m_CreationTime, Row++, 2 );
		m_Layout.AddWinAndEnable( &m_LastAccessTime, Row++, 2 );
#if defined(HAS_CHANGETIME)
		m_Layout.AddWinAndEnable( &m_ChangeTime, Row++, 2 );
#endif

		m_LastWriteDate.SetValidator( new clDateValidator() );
		m_LastWriteDate.SetReplaceMode( true );
		m_CreationDate.SetValidator( new clDateValidator() );
		m_CreationDate.SetReplaceMode( true );
		m_LastAccessDate.SetValidator( new clDateValidator() );
		m_LastAccessDate.SetReplaceMode( true );
#if defined(HAS_CHANGETIME)
		m_ChangeDate.SetValidator( new clDateValidator() );
		m_ChangeDate.SetReplaceMode( true );
#endif
		m_LastWriteTime.SetValidator( new clTimeValidator() );
		m_LastWriteTime.SetReplaceMode( true );
		m_CreationTime.SetValidator( new clTimeValidator() );
		m_CreationTime.SetReplaceMode( true );
		m_LastAccessTime.SetValidator( new clTimeValidator() );
		m_LastAccessTime.SetReplaceMode( true );
#if defined(HAS_CHANGETIME)
		m_ChangeTime.SetValidator( new clTimeValidator() );
		m_ChangeTime.SetReplaceMode( true );
#endif

		UpdateAttributes( m_Node );
		
		AddLayout( &m_Layout );

		order.clear();
#if defined(_WIN32)
		order.append(&m_ReadOnly);
		order.append(&m_Archive);
		order.append(&m_Hidden);
		order.append(&m_System);
#else
		order.append(&m_UserRead);
		order.append(&m_UserWrite);
		order.append(&m_UserExecute);
		order.append(&m_GroupRead);
		order.append(&m_GroupWrite);
		order.append(&m_GroupExecute);
		order.append(&m_OthersRead);
		order.append(&m_OthersWrite);
		order.append(&m_OthersExecute);
#endif // _WIN32
		order.append(&m_LastWriteDate);
		order.append(&m_LastWriteTime);
		order.append(&m_CreationDate);
		order.append(&m_CreationTime);
		order.append(&m_LastAccessDate);
		order.append(&m_LastAccessTime);
#if defined(HAS_CHANGETIME)
		order.append(&m_ChangeDate);
		order.append(&m_ChangeTime);
#endif // HAS_CHANGETIME

		SetPosition();
	}

	FSString GetURI() const { return m_URI; }

	FSNode* GetNode() const
	{
		if ( !m_Node ) return nullptr;

#if defined(_WIN32)
		m_Node->SetAttrReadOnly( m_ReadOnly.IsSet() );
		m_Node->SetAttrArchive( m_Archive.IsSet() );
		m_Node->SetAttrHidden( m_Hidden.IsSet() );
		m_Node->SetAttrSystem( m_System.IsSet() );
//		m_Node->SetAttrNotIndexed( m_NotIndexed.IsSet() );
//		m_Node->SetAttrTemporary( m_Temporary.IsSet() );
#else
		m_Node->SetAttrUserRead( m_UserRead.IsSet() );
		m_Node->SetAttrUserWrite( m_UserWrite.IsSet() );
		m_Node->SetAttrUserExecute( m_UserExecute.IsSet() );
		m_Node->SetAttrGroupRead( m_GroupRead.IsSet() );
		m_Node->SetAttrGroupWrite( m_GroupWrite.IsSet() );
		m_Node->SetAttrGroupExecute( m_GroupExecute.IsSet() );
		m_Node->SetAttrOthersRead( m_OthersRead.IsSet() );
		m_Node->SetAttrOthersWrite( m_OthersWrite.IsSet() );
		m_Node->SetAttrOthersExecute( m_OthersExecute.IsSet() );
#endif
		std::string LastWriteDate = m_LastWriteDate.GetTextStr();
		std::string LastWriteTime = m_LastWriteTime.GetTextStr();
		std::string CreationDate = m_CreationDate.GetTextStr();
		std::string CreationTime = m_CreationTime.GetTextStr();
		std::string LastAccessDate = m_LastAccessDate.GetTextStr();
		std::string LastAccessTime = m_LastAccessTime.GetTextStr();
		std::string ChangeDate = m_ChangeDate.GetTextStr();
		std::string ChangeTime = m_ChangeTime.GetTextStr();

		m_Node->SetLastWriteTime( GetFSTimeFromStr( LastWriteDate, LastWriteTime ) );
		m_Node->SetCreationTime( GetFSTimeFromStr( CreationDate, CreationTime ) );
		m_Node->SetLastAccessTime( GetFSTimeFromStr( LastAccessDate, LastAccessTime ) );
		m_Node->SetChangeTime( GetFSTimeFromStr( ChangeDate, ChangeTime ) );

		return m_Node;
	}

private:
	void UpdateAttributes( FSNode* Node )
	{
		if ( !Node ) return;
#if defined(_WIN32)
		m_ReadOnly.Change( Node->IsAttrReadOnly() );
		m_Archive.Change( Node->IsAttrArchive() );
		m_Hidden.Change( Node->IsAttrHidden() );
		m_System.Change( Node->IsAttrSystem() );
		m_Compressed.Change( Node->IsAttrCompressed() );
		m_Encrypted.Change( Node->IsAttrEncrypted() );
		m_NotIndexed.Change( Node->IsAttrNotIndexed() );
		m_Sparse.Change( Node->IsAttrSparse() );
		m_Temporary.Change( Node->IsAttrTemporary() );
		m_Offline.Change( Node->IsAttrOffline() );
		m_ReparsePoint.Change( Node->IsAttrReparsePoint() );
		m_Virtual.Change( Node->IsAttrVirtual() );
#else
		m_UserRead.Change( Node->IsAttrUserRead() );
		m_UserWrite.Change( Node->IsAttrUserWrite() );
		m_UserExecute.Change( Node->IsAttrUserExecute() );
		m_GroupRead.Change( Node->IsAttrGroupRead() );
		m_GroupWrite.Change( Node->IsAttrGroupWrite() );
		m_GroupExecute.Change( Node->IsAttrGroupExecute() );
		m_OthersRead.Change( Node->IsAttrOthersRead() );
		m_OthersWrite.Change( Node->IsAttrOthersWrite() );
		m_OthersExecute.Change( Node->IsAttrOthersExecute() );
#endif
		FSTime LastWriteTime = Node->GetLastWriteTime();
		FSTime CreationTime = Node->GetCreationTime();
		FSTime LastAccessTime = Node->GetLastAccessTime();
		FSTime ChangeTime = Node->GetChangeTime();

		m_LastWriteDate.SetText( GetFSTimeStrDate( LastWriteTime ) );
		m_CreationDate.SetText( GetFSTimeStrDate( CreationTime ) );
		m_LastAccessDate.SetText( GetFSTimeStrDate( LastAccessTime ) );
		m_ChangeDate.SetText( GetFSTimeStrDate( ChangeTime ) );
		
		m_LastWriteTime.SetText( GetFSTimeStrTime( LastWriteTime ) );
		m_CreationTime.SetText( GetFSTimeStrTime( CreationTime ) );
		m_LastAccessTime.SetText( GetFSTimeStrTime( LastAccessTime ) );
		m_ChangeTime.SetText( GetFSTimeStrTime( ChangeTime ) );
	}

private:
	PanelWin* m_Panel;
	FSNode* m_Node;
	FSString m_URI;

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
	SButton m_UserRead;
	SButton m_UserWrite;
	SButton m_UserExecute;
	SButton m_GroupRead;
	SButton m_GroupWrite;
	SButton m_GroupExecute;
	SButton m_OthersRead;
	SButton m_OthersWrite;
	SButton m_OthersExecute;
#endif
	// time
	StaticLine m_LastWriteTimeLabel;
	StaticLine m_CreationTimeLabel;
	StaticLine m_LastAccessTimeLabel;
	StaticLine m_ChangeTimeLabel;

	EditLine m_LastWriteDate;
	EditLine m_CreationDate;
	EditLine m_LastAccessDate;
	EditLine m_ChangeDate;

	EditLine m_LastWriteTime;
	EditLine m_CreationTime;
	EditLine m_LastAccessTime;
	EditLine m_ChangeTime;
};

bool FileAttributesDlg( NCDialogParent* Parent, PanelWin* Panel )
{
	clFileAttributesWin Dialog( Parent, Panel );
	Dialog.SetEnterCmd( 0 );

	if ( Dialog.DoModal( ) == CMD_OK )
	{
		FSNode* Node = Dialog.GetNode();

		// apply changes
		if ( Node )
		{
			FSPath fspath;
			FSString URI = Dialog.GetURI();
			clPtr<FS> fs = ParzeURI( URI.GetUnicode(), fspath, {} );
			if ( fs )
			{
				int Err = 0;
				FSCInfo Info;
				fs->StatSetAttr( fspath, &Node->st, &Err, &Info );
				if ( Err != 0 )
				{
					throw_msg( "Error setting file attributes: %s", fs->StrError( Err ).GetUtf8() );
				}
				fs->SetFileTime( fspath, Node->GetCreationTime(), Node->GetLastAccessTime(), Node->GetLastWriteTime(), &Err, &Info );
				if ( Err != 0 )
				{
					throw_msg("Error setting file date & time: %s", fs->StrError(Err).GetUtf8());
				}
			}
		}

		return true;
	}

	return false;
}

