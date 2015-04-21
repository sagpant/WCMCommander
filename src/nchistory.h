/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * wcm@linderdaum.com
 */

#pragma once

#include <wal.h>

using namespace wal;


#define EDIT_FIELD_SEARCH_TEXT			"search-text"
#define EDIT_FIELD_SEARCH_REPLACE_TEXT	"search-replace-text"

#define EDIT_FIELD_APPLY_COMMAND			"apply-command"
#define EDIT_FIELD_MAKE_FOLDER			"make-folder"
#define EDIT_FIELD_FILE_MASK				"file-mask"
#define EDIT_FIELD_FILE_EDIT				"file-edit"
#define EDIT_FIELD_FILE_TARGET			"file-target"

#define EDIT_FIELD_FTP_SERVER				"ftp-server"
#define EDIT_FIELD_FTP_USER				"ftp-user"
#define EDIT_FIELD_SFTP_SERVER			"sftp-server"
#define EDIT_FIELD_SFTP_USER				"sftp-user"
#define EDIT_FIELD_SMB_SERVER				"smb-server"
#define EDIT_FIELD_SMB_DOMAIN				"smb-domain"
#define EDIT_FIELD_SMB_USER				"smb-user"


typedef std::vector<std::vector<unicode_t>> HistCollect_t;

void LoadFieldsHistory();

void SaveFieldsHistory();

HistCollect_t* GetFieldHistCollect( const char* FieldName );

void AddFieldTextToHistory( const char* FieldName, const unicode_t* Txt );


class NCHistory
{
public:
	NCHistory(): m_List(), m_Current( -1 ) {}
	~NCHistory() {}

	void Clear();
	void Put( const unicode_t* str );
	void DeleteAll( const unicode_t* Str );

	size_t Count() const;
	const unicode_t* operator[] ( size_t n );

	const unicode_t* Prev();
	const unicode_t* Next();

	void ResetToLast() { m_Current = -1; }

private:
	HistCollect_t m_List;
	int m_Current;
};
