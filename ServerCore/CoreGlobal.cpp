#include "pch.h"
#include "CoreGlobal.h"
#include "ThreadManager.h"
#include "DeadLockProfiler.h"
#include "Memory.h"
#include "SocketUtils.h"
#include "SendBuffer.h"
#include "GlobalQueue.h"
#include "JobTimer.h"
#include "DBConnectionPool.h"
#include "ConsoleLog.h"

ThreadManager*			GThreadManager		= nullptr;
Memory*						GMemory						= nullptr;
SendBufferManager*	GSendBufferManager	= nullptr;
GlobalQueue*				GGlobalQueue				= nullptr;
JobTimer*					GJobTimer					= nullptr;
DeadLockProfiler*		GDeadLockProfiler		= nullptr;
//DBConnectionPool*	GDBConnectionPool	= nullptr;
//ConsoleLog*				GConsoleLogger			= nullptr;
std::shared_ptr<ConsoleLog>				GConsoleLogger			= nullptr;

/*
	[CoreGlobal]
	��� �Ŵ����� ���� ������ ���
*/
class CoreGlobal
{
public:
	CoreGlobal()
	{
		GThreadManager = new ThreadManager;
		GMemory = new Memory;
		GSendBufferManager = new SendBufferManager;
		GGlobalQueue = new GlobalQueue();
		GJobTimer = new JobTimer;
		GDeadLockProfiler = new DeadLockProfiler;
		//GDBConnectionPool = new DBConnectionPool;
		//GConsoleLogger = new ConsoleLog;
		GConsoleLogger = MakeShared<ConsoleLog>();

		//���� �ʱ�ȭ
		SocketUtils::Init();
	}

	~CoreGlobal()
	{
		delete GThreadManager;
		delete GMemory;
		delete GSendBufferManager;
		delete GGlobalQueue;
		delete GJobTimer;
		delete GDeadLockProfiler;
		//delete GDBConnectionPool;
		//delete GConsoleLogger;
		SocketUtils::Clear();
	}

protected:

private:

};

CoreGlobal GCoreGlobal;