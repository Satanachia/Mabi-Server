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
	//���� �۾� ����
	const int32 prevCount = _jobCount.fetch_add(1);
	_jobs.Push(job);

	//�� ó�� �۾��� ���� ��쿡��(JobQueue�� ó�� �۾��� ���� �������� ���)
	if (0 != prevCount)
		return;

	/*
		�Ʒ� Execute�Լ��� ���� �Ǵٽ� JobQueue::Push�� ���ƿ� �� ����
		�̷��� �ϳ��� �����尡 �������� JobQueue�� �� �����ϴ� ������ ���� �� �ִ�.
		�׷��� �ٸ� ��������� JobQueue�� �ϰ��� �ֱ⸸ �ϰ� �������� ���ϴ� ������ ����� �ִ�.
		(�ᱹ ��� �ϰ��� �ϳ��� ������θ� ������ ����)
		�� �κ��� ó���ϱ� ���� ���� �����尡 JobQueue�� ���������� Ȯ���Ѵ�.
	*/

	//�� �����尡 �̹� �������� JobQueue�� ������ ����
	if (nullptr == LCurrentJobQueue && false == pushOnly && false == _pushOnlyValue)
		Execute();

	// �����ִ� �ٸ� �����尡 �����ϵ��� �� JobQueue�� GlobalQueue�� �ѱ��
	// ���� ���ÿ����� shared_from_this()�� Room
	else
		GGlobalQueue->Push(shared_from_this());
}

void JobQueue::Execute()
{
	//�� ������� Execute������
	LCurrentJobQueue = this;

	while (true)
	{
		Vector<JobRef> jobs;
		_jobs.PopAll(jobs);

		const int32 jobCount = static_cast<int32>(jobs.size());
		for (int32 i = 0; i < jobCount; i++)
			jobs[i]->Execute();

		//_jobCount�� 0���� �� �������� ��Ȳ(���� �ϰ��� 0��)
		if (_jobCount.fetch_sub(jobCount) == jobCount)
		{
			//Execute ����
			LCurrentJobQueue = nullptr;
			return;
		}

		//�� �����忡 �ʹ� ���� �ϰ��� ������ ���� ����
		//���� �ð��� ������ Execute ����������
		//LEndTickCount�� DoWorkerJob �Լ����� �ð� �������־���
		const uint64 now = ::GetTickCount64();
		if (LEndTickCount < now)
			continue;

		LCurrentJobQueue = nullptr;

		// �����ִ� �ٸ� �����尡 �����ϵ��� �� JobQueue�� GlobalQueue�� �ѱ��
		GGlobalQueue->Push(shared_from_this());
		break;
	}
}
