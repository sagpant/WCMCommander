/*
* Part of Wal Commander GitHub Edition
* https://github.com/corporateshark/WalCommander
* walcommander@linderdaum.com
*/

#pragma once

#include <swl.h>

using namespace wal;


/**
* Edit line controll with history and autocomplete support.
*/
class NCEditLine : public ComboBox
{
private:
	const char* m_fieldName;
	bool m_autoMode;

	//void SetCBList(const unicode_t* txt);
	//void Clear();

public:
	NCEditLine( const char* fieldName, int nId, Win* parent, const unicode_t* txt,
		int cols, int rows, bool up, bool frame3d, bool nofocusframe, crect* rect = 0 );

	virtual ~NCEditLine()
	{
	}

	bool Command( int id, int subId, Win* win, void* d ) override;

	int UiGetClassId() override;

	bool OnOpenBox() override;

	void OnCloseBox() override;

	void UpdateHistory();
};
