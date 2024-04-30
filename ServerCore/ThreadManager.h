#pragma once
#include <thread>
#include <functional>

/*---------------------------------
				ThreadManager
---------------------------------*/

class ThreadManager
{
public:
	ThreadManager();
	~ThreadManager();

	ThreadManager(const ThreadManager& _Other) = delete;
	ThreadManager(ThreadManager&& _Other) noexcept = delete;
	ThreadManager& operator=(const ThreadManager& _Other) = delete;
	ThreadManager& operator=(const ThreadManager&& _Other) noexcept = delete;

	void Launch(std::function<void()> callback);
	void Join();

	static void InitTLS();
	static void DestroyTLS();

	//JobQueue::Push쪽에서 건내준 GlobalQueue를 꺼내서 처리
	static void DoGlobalQueueWork();
	//JobTimer에 저장되어 있던 일감들을 현재 시간과 비교하여 JobQueue에 Push
	static void DistributeReserveJobs();

private:
	Mutex									_lock;
	std::vector<std::thread>		_threads;
};

