/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * walcommander@linderdaum.com
 */

#include "operwin.h"

static Mutex operMutex; //блокировать при изменении operStopList, и при к threadId и tNode  в OperThreadWin !!!

static OperThreadNode* volatile operStopList = 0;


int OperThreadNode::CallBack( OperCallback f, void* data )
{
	MutexLock lock( &mutex );
	cbRet = -1;
	cbData = data;
	cbFunc = f;

	if ( !WinThreadSignal( 1 ) )
	{
		return -1;
	}

	cbCond.Wait( &mutex );
	return cbRet;
}

OperThreadNode::~OperThreadNode()
{
	if ( !stopped ) { fprintf( stderr, "!!! BUG ~OperThreadNode 1\n" ); }
}



void OperThreadWin::DBGPrintStoppingList()
{
	MutexLock lock1( &operMutex );

	OperThreadNode* p;

	for ( p = operStopList; p; p = p->next )
	{
		MutexLock lock2( &p->mutex );
		printf( "stopped thread %s\n", p->threadInfo.data() ? p->threadInfo.data() : "<empty info>" );
	}
}

void OperThreadWin::SetStopFlag()
{
	if ( tNode )
	{
		MutexLock lockNode( &tNode->mutex );
		tNode->stopped = true;
	}
}

void OperThreadWin::StopThread()
{
	MutexLock lock( &operMutex );

	if ( !tNode )
	{
		threadId = -1;
		return;
	}

	MutexLock lockNode( &tNode->mutex );
	tNode->stopped = true;
	tNode->data = 0;

	if ( !this->cbExecuted ) //!!!
	{
		tNode->cbRet = -1;
		tNode->cbCond.Signal(); // на всякий случай, вдруг сигнал о каллбаке послан, но сообщение еще до окна не дошло
	}

	tNode->win = 0;
	tNode->prev = 0;

	if ( operStopList ) { operStopList->prev = tNode; }

	tNode->next = operStopList;
	operStopList = tNode;
	tNode = 0;
	threadId = -1;
}

struct OperThreadParam: public iIntrusiveCounter
{
	OperThreadFunc func;
	OperThreadNode* node;
	void RunFunc() const
	{
		if ( this->func )
		{
			this->func( this->node );
		}
	}
};

void* __123___OperThread( void* param )
{
	if ( !param ) { return nullptr; }

	clPtr<OperThreadParam> pTp( ( OperThreadParam* )param );

	// release the reference added before thread start
	pTp->DecRefCount();

	try
	{
		pTp->RunFunc();
	}
	catch ( ... )
	{
		fprintf( stderr, "__123___OperThread(): exception in OperThread!!!\n" );
	}

	MutexLock lock( &operMutex );
	MutexLock lockNode( &pTp->node->mutex );

	if ( pTp->node->stopped )
	{
		if ( pTp->node->prev )
		{
			pTp->node->prev->next = pTp->node->next;
		}
		else
		{
			operStopList = pTp->node->next;
		}

		if ( pTp->node->next )
		{
			pTp->node->next->prev = pTp->node->prev;
		}
	}
	else
	{
		ASSERT( pTp->node->win );
	}

	if ( pTp->node->win )
	{
		// reset pointer to the Node in corresponding ThreadWin since we are going to delete it
		pTp->node->win->tNode = nullptr;
	}
	
	pTp->node->stopped = true; //!!!
	lockNode.Unlock(); //!!!

#ifdef _DEBUG
	printf( "stop: %s\n", pTp->node->threadInfo.data( ) );
#endif

	delete( pTp->node );

	pTp->node = nullptr;

	return nullptr;
}



void OperThreadWin::RunNewThread( const char* info, OperThreadFunc f, void* data )
{
	StopThread();

	clPtr<OperThreadParam> param = new OperThreadParam;
	tNode = new OperThreadNode( this, info, data );

	param->func = f;
	param->node = tNode;

	MutexLock lock( &operMutex );

	try
	{
		int n = NewThreadID();
//printf("TN=%i\n", n);
		param->IncRefCount();
		ThreadCreate( n, __123___OperThread, param.ptr() );
		threadId = n;
		param.drop(); //!!!
	}
	catch ( ... )
	{
		delete tNode;
		tNode = 0;
	}
}

void OperThreadWin::ThreadSignal( int id, int data )
{
	if ( data == 1 )
	{
		MutexLock lock( &operMutex );

		if ( !tNode ) { return; } //уже остановлен и каллбаку послан согнал с отрицательным результатом

		ASSERT( !cbExecuted );
		cbExecuted = true;
		OperThreadNode* p = tNode;
		lock.Unlock();

		try
		{
			p->cbRet = p->cbFunc( p->cbData );
		}
		catch ( ... )
		{
			fprintf( stderr, "!!! exception in OperThreadWin::ThreadSignal !!!\n" );
		}

		lock.Lock();
		cbExecuted = false;
		p->cbCond.Signal();
	}
	else
	{
		OperThreadSignal( data );
	}
}

void OperThreadWin::OperThreadSignal( int data ) {}
void OperThreadWin::OperThreadStopped() {}

void OperThreadWin::ThreadStopped( int id, void* data )
{
	MutexLock lock( &operMutex );

//printf("stopped TN=%i\n", id);
	if ( threadId == id )
	{
		threadId = -1;
		lock.Unlock(); //!!!!!!!
		OperThreadStopped();
	}
}

OperThreadWin::~OperThreadWin()
{
	if ( cbExecuted ) { fprintf( stderr, "!!! BUG ~OperThreadWin\n" ); }

	StopThread();
}
