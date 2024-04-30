#pragma once
#include "Job.h"
#include "JobTimer.h"

/*---------------------------------------------------
				JobQueue
---------------------------------------------------*/

//�� Ŭ������ ��ӹ��� ��ü�� JobQueue ����� �̿��ؼ� ��Ŷ�� ó���Ѵ�(�ۼ���)
class JobQueue : public std::enable_shared_from_this<JobQueue>
{
public:
	JobQueue();
	~JobQueue();

	JobQueue(const JobQueue& _Other) = delete;
	JobQueue(JobQueue&& _Other) noexcept = delete;
	JobQueue& operator=(const JobQueue& _Other) = delete;
	JobQueue& operator=(const JobQueue&& _Other) noexcept = delete;

	//����Լ��� ������ �ݹ��� �޾� Job�� �����ϰ� JobQueue�� �����Ѵ�. ���� ���� ó���� �� ������ ó���Ѵ�
	void DoASync(CallbackType&& callback)
	{
		Push(ObjectPool<Job>::MakeShared(std::move(callback)));
	}

	//����Լ� �ݹ��� �޾� Job�� �����ϰ� JobQueue�� �����Ѵ�. ���� ���� ó���� �� ������ ó���Ѵ�
	template <typename T, typename Ret, typename... Args>
	void DoASync(Ret(T::* memFunc)(Args...), Args... args)
	{
		std::shared_ptr<T> owner = std::static_pointer_cast<T>(shared_from_this());
		Push(ObjectPool<Job>::MakeShared(owner, memFunc, std::forward<Args>(args)...));
	}

	//tickAfter�и������� �ڿ� JobQueue�� �ϰ� �ֱ�
	void DoTimer(uint64 tickAfter, CallbackType&& callback)
	{
		JobRef job = ObjectPool<Job>::MakeShared(std::move(callback));
		GJobTimer->Reserve(tickAfter, shared_from_this(), job);
	}

	//tickAfter�и������� �ڿ� JobQueue�� �ϰ� �ֱ�
	template <typename T, typename Ret, typename... Args>
	void DoTimer(uint64 tickAfter, Ret(T::* memFunc)(Args...), Args... args)
	{
		std::shared_ptr<T> owner = std::static_pointer_cast<T>(shared_from_this());
		JobRef job = ObjectPool<Job>::MakeShared(owner, memFunc, std::forward<Args>(args)...);
		GJobTimer->Reserve(tickAfter, shared_from_this(), job);
	}

	
	//���� ĸ�ķ� �����ߴ� shared_ptr�� ��ȯ������ �����Ѵ�
	void ClearJobs() { _jobs.Clear(); }

	void Execute();

	void Push(JobRef job, bool pushOnly = false);

protected:
	LockQueue<JobRef>	_jobs;
	//���� �������� �۾� ����
	Atomic<int32>				_jobCount = 0;
	
	bool _pushOnlyValue = false;
};

