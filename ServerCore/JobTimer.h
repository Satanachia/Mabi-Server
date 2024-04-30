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
		//�ּ���
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

	//tickAfter�и������� �ڿ� ó���� �ϰ��� ����
	void Reserve(uint64 tickAfter, std::weak_ptr<JobQueue> owner, JobRef job);
	//������ �ϰ����� �ð��� �� �ƴٸ� JobQueue�� �־��ش�.(� JobQueue�� �־��� ���� Reserve ���ڿ��� �־��־���)
	void Distribute(uint64 now);
	void Clear();

private:
	USE_LOCK;

	PriorityQueue<TimerItem>	_items;
	Atomic<bool>						_distributing = false;
};

