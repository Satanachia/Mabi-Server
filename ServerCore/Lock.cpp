#include "pch.h"
#include "Lock.h"
#include "DeadLockProfiler.h"

Lock::Lock()
{

}

Lock::~Lock()
{

}


void Lock::WriteLock(const char* name)
{
#ifdef _DEBUG
	GDeadLockProfiler->PushLock(name);
#endif // DEBUG


	//���� Write���� ��� �ִ� ������ ���̵�
	const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	
	//������ �����尡 Lock�� ��������� ����� ��
	// ũ���ø� ���� �ʰ� _writeCount�� ������Ŵ
	if (LThreadId == lockThreadId)
	{
		++_writeCount;
		return;
	}


	//������ ������ �ð�
	const int64 beginTick = ::GetTickCount64();
	const uint32 desired = (LThreadId << 16) & WRITE_THREAD_MASK;

	//�ƹ��� (Write)����,(Read)���� �ϰ� ���� ���� ��, �����ؼ� �������� ��´�.
	while (true)
	{
		//MAX_SPIN_COUNT��ŭ ����
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; ++spinCount)
		{
			uint32 expected = EMPTY_FLAG;
			if (false == _lockFlag.compare_exchange_strong(OUT expected, desired))
				continue;

			++_writeCount;
			return;
		}

		//�ʹ� ���� �ð����� �������� ���� ���� ���
		if (ACQUIRE_TIMEOUT_TICK <= (::GetTickCount64() - beginTick))
		{
			//��������� �Ǵ��ؼ� �Ͷ߸���
			CRASH("LOCK_TIME_OUT");
		}
		
		//������ �ΰ���
		std::this_thread::yield();
	}
}

void Lock::WriteUnlock(const char* name)
{
#ifdef _DEBUG
	GDeadLockProfiler->PopLock(name);
#endif // DEBUG

	//�Ͼ ���� �������� Ȥ�ó� ���׸� ���� ����
	if (0 != (_lockFlag.load() & READ_COUNT_MASK))
	{
		/*
			������ �����忡�� Write���� ��� Read���� ��°� ����
			������ Read���� ���� ��� Write���� ��°� ��å�� ������� �ʾ���

			Read��°� �ٸ�  �����嵵 ������ ����Ѵٴ� �ǹ��ε�,
			Read��� Write���� ��°� write���� ������ ���� ����
			�̷� ��� ���� write���� ��� read���� ��� ������ �����

			Read���� ��´ٴ� ���� ���Ŀ� Write���� ���� �ʴ´ٴ� �ǹ�
		*/

		//ReadLock�� �� Ǯ�� ������ WriteUnlock�� �Ұ���
		CRASH("INVALID_UNLOCK_MASK");
	}

	const int32 lockCount = --_writeCount;
	if (0 < lockCount)
		return;

	_lockFlag.store(EMPTY_FLAG);
}

void Lock::ReadLock(const char* name)
{
#ifdef _DEBUG
	GDeadLockProfiler->PushLock(name);
#endif // DEBUG

	//���� Write���� ��� �ִ� ������ ���̵�
	const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;

	//������ �����尡 �� Write���� �����ϰ� �־��ٸ� ������ ����
	if (LThreadId == lockThreadId)
	{
		_lockFlag.fetch_add(1);
		return;
	}


	//������ ������ �ð�
	const int64 beginTick = ::GetTickCount64();

	//�ƹ��� Write���� �����ϰ� ���� �ʴٸ� ���� ī��Ʈ�� �ø���
	while (true)
	{
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; ++spinCount)
		{
			//Write�� ��Ʈ�� 0���� �� �и��鼭
			//�������� ���� ��쿣 1 ������Ŵ -> ReadLock����
			uint32 expected = _lockFlag.load() & READ_COUNT_MASK;
			if (true == _lockFlag.compare_exchange_strong(expected, expected + 1))
				return;
		}

		//�ʹ� ���� �ð����� �������� ���� ���� ���
		if (ACQUIRE_TIMEOUT_TICK <= (::GetTickCount64() - beginTick))
		{
			//��������� �Ǵ��ؼ� �Ͷ߸���
			CRASH("LOCK_TIME_OUT");
		}

		std::this_thread::yield();
	}
}

void Lock::ReadUnlock(const char* name)
{
#ifdef _DEBUG
	GDeadLockProfiler->PopLock(name);
#endif // DEBUG

	/*
	* fetch_sub�� -1�� ���������� ����������
	* ��ȯ���� -1�� �����ϱ� �������� ����
	*/
	if (0 == (_lockFlag.fetch_sub(1) & READ_COUNT_MASK))
	{
		CRASH("MULTIPLE_UNLOCK");
	}
}
