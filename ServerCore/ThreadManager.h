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

	//JobQueue::Push�ʿ��� �ǳ��� GlobalQueue�� ������ ó��
	static void DoGlobalQueueWork();
	//JobTimer�� ����Ǿ� �ִ� �ϰ����� ���� �ð��� ���Ͽ� JobQueue�� Push
	static void DistributeReserveJobs();

private:
	Mutex									_lock;
	std::vector<std::thread>		_threads;
};

