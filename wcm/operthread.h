/*
	Copyright (c) by Valery Goryachev (Wal)
*/


#ifndef OPERTHREAD_H
#define OPERTHREAD_H

#include "ncdialogs.h"
#include "operwin.h"

class OperData {
	NCDialogParent * volatile parentWin;
public:
	OperData(NCDialogParent *p):parentWin(p){}
	NCDialogParent* Parent(){ return parentWin; }
	virtual ~OperData();
};

extern ButtonDataNode bOk[];

class OperThread {
	OperThreadNode * volatile node;
	carray<char> operName; //++volatile
	NCDialogParent * volatile parentWin;
public:
	OperThread(const char *opName, NCDialogParent *p, OperThreadNode *n);
	OperThreadNode& Node(){ return *node; }
	const char *OperName(){ return operName.ptr(); }
	virtual void Run();
	virtual ~OperThread();
protected:	
	NCDialogParent* Parent() { return parentWin; }
	int RedMessage(ButtonDataNode * b, const char *str, const char *sysErr = 0);
	int RedMessage(const char *s1, ButtonDataNode * buttons = bOk, const char *sysErr = 0);
	int RedMessage(const char *s1, const char *s2, ButtonDataNode * buttons = bOk, const char *sysErr = 0);
	int RedMessage(const char *s1, const char *s2, const char *s3, ButtonDataNode * buttons = bOk, const char *sysErr = 0);
	int RedMessage(const char *s1, const char *s2, const char *s3, const char *s4, ButtonDataNode * buttons = bOk, const char *sysErr = 0);
	int RedMessage(const char *s1, const char *s2, const char *s3, const char *s4, const char *s5, ButtonDataNode * buttons = bOk, const char *sysErr = 0);
	int RedMessage(const char *s1, const char *s2, const char *s3, const char *s4, const char *s5, const char *s6, ButtonDataNode * buttons = bOk, const char *sysErr = 0);
	int RedMessage(const char *s1, const char *s2, const char *s3, const char *s4, const char *s5, const char *s6, const char *s7, ButtonDataNode * buttons = bOk, const char *sysErr = 0);
};



#endif
