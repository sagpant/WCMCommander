/*
   Copyright (c) by Valery Goryachev (Wal)
*/


#include "operthread.h"
#include "string-util.h"

ButtonDataNode bOk[] = { { " Ok ", CMD_OK},  {0, 0}};

OperData::~OperData() {}

OperThread::OperThread( const char* opName, NCDialogParent* p, OperThreadNode* n )
	:  operName( new_char_str( opName ) ),
	   node( n ),
	   parentWin( p )
{
	//printf("OperThread create\n");
}

void OperThread::Run() {}
OperThread::~OperThread()
{
	//printf("OperThread DESTROY\n");
}

struct CbRedMsgData
{
	const char* volatile message;
	const char* volatile operName;
	ButtonDataNode* volatile buttons;
	NCDialogParent* volatile parent;
};

static int RedCallBack( void* cbData )
{
	CbRedMsgData& param = *( ( CbRedMsgData* )cbData );
	return NCMessageBox( param.parent, param.operName, param.message, true, param.buttons );
}

int OperThread::RedMessage( ButtonDataNode* b, const char* str, const char* sysErr )
{
	std::vector<char> msg = sysErr ? carray_cat<char>( str, ":\n", sysErr ) : new_char_str( str );
	CbRedMsgData cbParam;
	cbParam.message = msg.data();
	cbParam.operName = OperName();
	cbParam.buttons = b;
	cbParam.parent = Parent();
	return Node().CallBack( RedCallBack, &cbParam );
}

int OperThread::RedMessage( const char* s1, ButtonDataNode* buttons, const char* sysErr )
{ return RedMessage( buttons, new_char_str( s1 ).data(), sysErr ); }

int OperThread::RedMessage( const char* s1, const char* s2, ButtonDataNode* buttons, const char* sysErr )
{
	return RedMessage( buttons, carray_cat<char>( s1, s2 ).data(), sysErr );
}

int OperThread::RedMessage( const char* s1, const char* s2, const char* s3, ButtonDataNode* buttons, const char* sysErr )
{
	return RedMessage( buttons, carray_cat<char>( s1, s2, s3 ).data(), sysErr );
}

int OperThread::RedMessage( const char* s1, const char* s2, const char* s3, const char* s4, ButtonDataNode* buttons, const char* sysErr )
{
	return RedMessage( buttons, carray_cat<char>( s1, s2, s3, s4 ).data(), sysErr );
}

int OperThread::RedMessage( const char* s1, const char* s2, const char* s3, const char* s4, const char* s5, ButtonDataNode* buttons, const char* sysErr )
{
	return RedMessage( buttons, carray_cat<char>( s1, s2, s3, s5, s5 ).data(), sysErr );
}

int OperThread::RedMessage( const char* s1, const char* s2, const char* s3, const char* s4, const char* s5,  const char* s6, ButtonDataNode* buttons, const char* sysErr )
{
	return RedMessage( buttons, carray_cat<char>( s1, s2, s3, s5, s5, s6 ).data(), sysErr );
}

int OperThread::RedMessage( const char* s1, const char* s2, const char* s3, const char* s4, const char* s5,  const char* s6, const char* s7, ButtonDataNode* buttons, const char* sysErr )
{
	return RedMessage( buttons, carray_cat<char>( s1, s2, s3, s5, s5, s6, s7 ).data(), sysErr );
}

