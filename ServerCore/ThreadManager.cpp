#include "pch.h"
#include "ThreadManager.h"
#include "CoreTLS.h"
#include "CoreGlobal.h"
#include "GlobalQueue.h"
#include "JobQueue.h"
#include "JobTimer.h"

ThreadManager::ThreadManager()
{
	//메인 스레드 TLS 초기화
	InitTLS();
}


ThreadManager::~ThreadManager()
{
	Join();
}

void ThreadManager::Launch(std::function<void()> callback)
{
	LockGuard gurad(_lock);

	_threads.push_back(std::thread([=]()
	{
		InitTLS();
		callback();
		DestroyTLS();
	}));
}

void ThreadManager::Join()
{
	for (std::thread& t : _threads)
	{
		if (true == t.joinable())
			t.join();
	}

	_threads.clear();
}


void ThreadManager::InitTLS()
{
	static Atomic<uint32> SThreadId = 1;
	LThreadId = SThreadId.fetch_add(1);

	//TODO
}

void ThreadManager::DestroyTLS()
{
	//TODO
}

void ThreadManager::DoGlobalQueueWork()
{
	while (true)
	{
		//일정 시간이 지나면 Execute 빠져나오기
		//LEndTickCount는 DoWorkerJob 함수에서 시간 설정해주었음
		uint64 now = ::GetTickCount64();
		if (LEndTickCount < now)
			break;

		JobQueueRef jobQueue = GGlobalQueue->Pop();
		if (nullptr == jobQueue)
			break;

		jobQueue->Execute();
	}
}

void ThreadManager::DistributeReserveJobs()
{
	const uint64 now = ::GetTickCount64();
	GJobTimer->Distribute(now);
}



