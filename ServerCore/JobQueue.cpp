#include "pch.h"
#include "JobQueue.h"
#include "GlobalQueue.h"

/*---------------------------------------------------
				JobQueue
---------------------------------------------------*/

JobQueue::JobQueue()
{

}

JobQueue::~JobQueue()
{

}

void JobQueue::Push(JobRef job, bool pushOnly /*= false*/)
{
	//이전 작업 갯수
	const int32 prevCount = _jobCount.fetch_add(1);
	_jobs.Push(job);

	//맨 처음 작업을 넣은 경우에만(JobQueue에 처음 작업을 넣은 쓰레드인 경우)
	if (0 != prevCount)
		return;

	/*
		아래 Execute함수를 통해 또다시 JobQueue::Push로 돌아올 수 있음
		이러면 하나의 스레드가 여러개의 JobQueue를 다 선점하는 문제가 생길 수 있다.
		그러면 다른 스레드들은 JobQueue에 일감을 넣기만 하고 실행하지 못하는 문제가 생길수 있다.
		(결국 모든 일감이 하나의 스레드로만 몰리는 문제)
		이 부분을 처리하기 위해 현재 스레드가 JobQueue를 실행중인지 확인한다.
	*/

	//이 스레드가 이미 실행중인 JobQueue가 없으면 실행
	if (nullptr == LCurrentJobQueue && false == pushOnly && false == _pushOnlyValue)
		Execute();

	// 여유있는 다른 스레드가 실행하도록 이 JobQueue를 GlobalQueue에 넘긴다
	// 지금 예시에서는 shared_from_this()는 Room
	else
		GGlobalQueue->Push(shared_from_this());
}

void JobQueue::Execute()
{
	//이 스레드는 Execute실행중
	LCurrentJobQueue = this;

	while (true)
	{
		Vector<JobRef> jobs;
		_jobs.PopAll(jobs);

		const int32 jobCount = static_cast<int32>(jobs.size());
		for (int32 i = 0; i < jobCount; i++)
			jobs[i]->Execute();

		//_jobCount가 0으로 딱 떨어지는 상황(남은 일감이 0개)
		if (_jobCount.fetch_sub(jobCount) == jobCount)
		{
			//Execute 종료
			LCurrentJobQueue = nullptr;
			return;
		}

		//이 스레드에 너무 많은 일감이 몰리는 것을 방지
		//일정 시간이 지나면 Execute 빠져나오기
		//LEndTickCount는 DoWorkerJob 함수에서 시간 설정해주었음
		const uint64 now = ::GetTickCount64();
		if (LEndTickCount < now)
			continue;

		LCurrentJobQueue = nullptr;

		// 여유있는 다른 스레드가 실행하도록 이 JobQueue를 GlobalQueue에 넘긴다
		GGlobalQueue->Push(shared_from_this());
		break;
	}
}
