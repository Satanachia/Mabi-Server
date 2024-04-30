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


	//현재 Write락을 잡고 있는 스레드 아이디
	const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	
	//동일한 스레드가 Lock을 재귀적으로 잡았을 때
	// 크래시를 내지 않고 _writeCount를 증가시킴
	if (LThreadId == lockThreadId)
	{
		++_writeCount;
		return;
	}


	//경쟁을 시작한 시간
	const int64 beginTick = ::GetTickCount64();
	const uint32 desired = (LThreadId << 16) & WRITE_THREAD_MASK;

	//아무도 (Write)소유,(Read)공유 하고 있지 않을 때, 경합해서 소유권을 얻는다.
	while (true)
	{
		//MAX_SPIN_COUNT만큼 스핀
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; ++spinCount)
		{
			uint32 expected = EMPTY_FLAG;
			if (false == _lockFlag.compare_exchange_strong(OUT expected, desired))
				continue;

			++_writeCount;
			return;
		}

		//너무 오랜 시간동안 소유권을 잡지 못한 경우
		if (ACQUIRE_TIMEOUT_TICK <= (::GetTickCount64() - beginTick))
		{
			//데드락으로 판단해서 터뜨리기
			CRASH("LOCK_TIME_OUT");
		}
		
		//다음에 두고보자
		std::this_thread::yield();
	}
}

void Lock::WriteUnlock(const char* name)
{
#ifdef _DEBUG
	GDeadLockProfiler->PopLock(name);
#endif // DEBUG

	//일어날 일이 없겠지만 혹시나 버그를 위해 넣음
	if (0 != (_lockFlag.load() & READ_COUNT_MASK))
	{
		/*
			동일한 스레드에서 Write락을 잡고 Read락을 잡는건 허용됨
			하지만 Read락을 먼저 잡고 Write락을 잡는건 정책상 허락하지 않았음

			Read잡는게 다른  스레드도 접근을 허용한다는 의미인데,
			Read잡고 Write락을 잡는건 write락을 잡을수 없는 조건
			이런 경우 차라리 write락을 잡고 read락을 잡는 것으로 허락함

			Read락을 잡는다는 것은 이후에 Write락을 하지 않는다는 의미
		*/

		//ReadLock을 다 풀기 전에는 WriteUnlock은 불가능
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

	//현재 Write락을 잡고 있는 스레드 아이디
	const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;

	//동일한 스레드가 이 Write락을 소유하고 있었다면 무조건 성공
	if (LThreadId == lockThreadId)
	{
		_lockFlag.fetch_add(1);
		return;
	}


	//경쟁을 시작한 시간
	const int64 beginTick = ::GetTickCount64();

	//아무도 Write락을 소유하고 있지 않다면 공유 카운트를 올린다
	while (true)
	{
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; ++spinCount)
		{
			//Write쪽 비트는 0으로 싹 밀리면서
			//기존값과 같은 경우엔 1 증가시킴 -> ReadLock잡음
			uint32 expected = _lockFlag.load() & READ_COUNT_MASK;
			if (true == _lockFlag.compare_exchange_strong(expected, expected + 1))
				return;
		}

		//너무 오랜 시간동안 소유권을 잡지 못한 경우
		if (ACQUIRE_TIMEOUT_TICK <= (::GetTickCount64() - beginTick))
		{
			//데드락으로 판단해서 터뜨리기
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
	* fetch_sub은 -1을 원자적으로 수행하지만
	* 반환값은 -1을 수행하기 이전값이 나옴
	*/
	if (0 == (_lockFlag.fetch_sub(1) & READ_COUNT_MASK))
	{
		CRASH("MULTIPLE_UNLOCK");
	}
}
