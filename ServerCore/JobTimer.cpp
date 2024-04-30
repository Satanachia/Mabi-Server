#include "pch.h"
#include "JobTimer.h"
#include "JobQueue.h"

/*----------------------------------
					JobTimer
----------------------------------*/

JobTimer::JobTimer()
{

}

JobTimer::~JobTimer()
{

}

void JobTimer::Reserve(uint64 tickAfter, std::weak_ptr<JobQueue> owner, JobRef job)
{
	const uint64 executeTick = ::GetTickCount64() + tickAfter;
	JobData* jobData = ObjectPool<JobData>::Pop(owner, job);

	WRITE_LOCK;
	_items.push(TimerItem{ executeTick, jobData });
}

void JobTimer::Distribute(uint64 now)
{
	//하나의 스레드만 Distribute를 처리
	/*
		이렇게 하나의 스레드만 처리하게 시키는 이유는,
		A스레드가 Vector<TimerItem> items로 Job들을 옮겨놓고
		어떤 이유로 렉이 걸린 이후에
		B스레드로 컨텍스트 스위칭이 일어나면
		렉이 걸렸기 때문에 	A스레드의 Job보다 B스레드의 Job이 먼저 실행될 수도 있다
		이 부분을 아예 방지하기 위함
	*/
	if (true == _distributing.exchange(true))
		return;

	//지금 처리해야 할 일감들을 벡터로 옮기기
	Vector<TimerItem> items;
	{
		WRITE_LOCK;

		while (false == _items.empty())
		{
			const TimerItem& timerItem = _items.top();
			if (now < timerItem.executeTick)
				break;

			items.push_back(timerItem);
			_items.pop();
		}
	}

	for (TimerItem& item : items)
	{
		JobQueueRef owner = item.jobData->_owner.lock();

		//해당 JobQueue가 유효하다면
		//일감을 처리하기 위해 JobQueue에 Push
		if (nullptr != owner)
			owner->Push(item.jobData->_job, true);

		ObjectPool<JobData>::Push(item.jobData);
	}
	
	_distributing.store(false);
}

void JobTimer::Clear()
{
	WRITE_LOCK;

	while (false == _items.empty())
	{
		const TimerItem& timerItem = _items.top();
		ObjectPool<JobData>::Push(timerItem.jobData);
		_items.pop();
	}
}
