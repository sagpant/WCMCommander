/*
* Part of WCM Commander
* https://github.com/corporateshark/WCMCommander
* walcommander@linderdaum.com
*/

#pragma once

#include <swl.h>

using namespace wal;


/**
 * Edit line controll with history and autocomplete support.
 */
class clNCEditLine : public ComboBox
{
private:
	std::vector<unicode_t> m_Prefix;
	const char* m_FieldName;
	
public:
	clNCEditLine( const char* FieldName, int Id, Win* Parent, const unicode_t* Txt, int Cols, int Rows, crect* Rect = 0 );

	virtual ~clNCEditLine() {}

	virtual bool EventKey( cevent_key* pEvent ) override;

	virtual bool Command( int Id, int SubId, Win* Win, void* Data ) override;

	virtual int UiGetClassId() override;

	virtual bool OnOpenBox() override;
	
	virtual void OnItemChanged( int ItemIndex ) override;

	void AddCurrentTextToHistory();
	
private:
	void InitBox();
};
