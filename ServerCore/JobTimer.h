#pragma once

struct JobData
{
	JobData(std::weak_ptr<JobQueue> owner, JobRef job)
		:_owner(owner)
		, _job(job)
	{
	}

	std::weak_ptr<JobQueue> _owner;
	JobRef _job;
};

struct TimerItem
{
	bool operator<(const TimerItem& other) const
	{
		//최소힙
		return (executeTick > other.executeTick);
	}

	uint64 executeTick = 0;
	JobData* jobData = nullptr;
};


/*----------------------------------
					JobTimer
----------------------------------*/

class JobTimer
{
public:
	JobTimer();
	~JobTimer();

	JobTimer(const JobTimer& _Other) = delete;
	JobTimer(JobTimer&& _Other) noexcept = delete;
	JobTimer& operator=(const JobTimer& _Other) = delete;
	JobTimer& operator=(const JobTimer&& _Other) noexcept = delete;

	//tickAfter밀리세컨드 뒤에 처리할 일감을 예약
	void Reserve(uint64 tickAfter, std::weak_ptr<JobQueue> owner, JobRef job);
	//예약한 일감들이 시간이 다 됐다면 JobQueue에 넣어준다.(어떤 JobQueue에 넣어줄 지는 Reserve 인자에서 넣어주었음)
	void Distribute(uint64 now);
	void Clear();

private:
	USE_LOCK;

	PriorityQueue<TimerItem>	_items;
	Atomic<bool>						_distributing = false;
};

