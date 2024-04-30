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
	//�ϳ��� �����常 Distribute�� ó��
	/*
		�̷��� �ϳ��� �����常 ó���ϰ� ��Ű�� ������,
		A�����尡 Vector<TimerItem> items�� Job���� �Űܳ���
		� ������ ���� �ɸ� ���Ŀ�
		B������� ���ؽ�Ʈ ����Ī�� �Ͼ��
		���� �ɷȱ� ������ 	A�������� Job���� B�������� Job�� ���� ����� ���� �ִ�
		�� �κ��� �ƿ� �����ϱ� ����
	*/
	if (true == _distributing.exchange(true))
		return;

	//���� ó���ؾ� �� �ϰ����� ���ͷ� �ű��
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

		//�ش� JobQueue�� ��ȿ�ϴٸ�
		//�ϰ��� ó���ϱ� ���� JobQueue�� Push
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
