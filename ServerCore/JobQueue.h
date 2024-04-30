#pragma once
#include "Job.h"
#include "JobTimer.h"

/*---------------------------------------------------
				JobQueue
---------------------------------------------------*/

//이 클래스를 상속받은 객체는 JobQueue 방식을 이용해서 패킷을 처리한다(송수신)
class JobQueue : public std::enable_shared_from_this<JobQueue>
{
public:
	JobQueue();
	~JobQueue();

	JobQueue(const JobQueue& _Other) = delete;
	JobQueue(JobQueue&& _Other) noexcept = delete;
	JobQueue& operator=(const JobQueue& _Other) = delete;
	JobQueue& operator=(const JobQueue&& _Other) noexcept = delete;

	//멤버함수를 제외한 콜백을 받아 Job을 생성하고 JobQueue에 보관한다. 만약 일을 처리할 수 있으면 처리한다
	void DoASync(CallbackType&& callback)
	{
		Push(ObjectPool<Job>::MakeShared(std::move(callback)));
	}

	//멤버함수 콜백을 받아 Job을 생성하고 JobQueue에 보관한다. 만약 일을 처리할 수 있으면 처리한다
	template <typename T, typename Ret, typename... Args>
	void DoASync(Ret(T::* memFunc)(Args...), Args... args)
	{
		std::shared_ptr<T> owner = std::static_pointer_cast<T>(shared_from_this());
		Push(ObjectPool<Job>::MakeShared(owner, memFunc, std::forward<Args>(args)...));
	}

	//tickAfter밀리세컨드 뒤에 JobQueue에 일감 넣기
	void DoTimer(uint64 tickAfter, CallbackType&& callback)
	{
		JobRef job = ObjectPool<Job>::MakeShared(std::move(callback));
		GJobTimer->Reserve(tickAfter, shared_from_this(), job);
	}

	//tickAfter밀리세컨드 뒤에 JobQueue에 일감 넣기
	template <typename T, typename Ret, typename... Args>
	void DoTimer(uint64 tickAfter, Ret(T::* memFunc)(Args...), Args... args)
	{
		std::shared_ptr<T> owner = std::static_pointer_cast<T>(shared_from_this());
		JobRef job = ObjectPool<Job>::MakeShared(owner, memFunc, std::forward<Args>(args)...);
		GJobTimer->Reserve(tickAfter, shared_from_this(), job);
	}

	
	//람다 캡쳐로 저장했던 shared_ptr의 순환참조를 제거한다
	void ClearJobs() { _jobs.Clear(); }

	void Execute();

	void Push(JobRef job, bool pushOnly = false);

protected:
	LockQueue<JobRef>	_jobs;
	//현재 실행중인 작업 갯수
	Atomic<int32>				_jobCount = 0;
	
	bool _pushOnlyValue = false;
};

