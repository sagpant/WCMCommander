/*
* Part of Wal Commander GitHub Edition
* https://github.com/corporateshark/WalCommander
* walcommander@linderdaum.com
*/

#pragma once

#include <swl.h>

using namespace wal;

typedef ccollect<std::vector<unicode_t>> HistCollect;


// editline with history and autocomplete
class NCEditLine: public ComboBox {

private:
    clPtr<HistCollect> _histList;
	const char *_group;
	bool _autoMode;
	void SetCBList(const unicode_t *txt);
	void LoadHistoryList();

public:
	NCEditLine(const char *acGroup, int nId, Win *parent, const unicode_t *txt, int cols, int rows, bool up, bool frame3d, bool nofocusframe, crect *rect = 0);
	
    virtual ~NCEditLine() {}

    virtual bool Command(int id, int subId, Win *win, void *d);
	
    virtual int UiGetClassId();
	
    virtual bool OnOpenBox();
	
    virtual void OnCloseBox();
	
    void Clear();

    void Commit();
};


HistCollect* HistGetList(const char *histGroup);
