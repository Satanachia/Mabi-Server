#pragma once

/*----------------------------------------------------
						Read&Write Spin Lock
----------------------------------------------------*/

/*
	Read&Write Lock필요성
		수정이 없는 고정된 데이터는 Lock을 잡을 필요 없음
		그런데 정말 특별하게 수정이 필요한 경우가 생길수 있음
		그 특별한 경우 때문에 평소에도 Lock을 잡는건 너무 아까움
		이럴때 사용할 수 있는 Lock이 Read&Write 
*/

/*
	총 32비트를 사용함
	상위 16비트는 Write를 할때 그 락을 얻은 스레드의 ID을 의미함
	하위 16비트는 Read할때 락을 사용중인 스레드의 갯수를 의미함
*/

class Lock
{
	enum : uint32
	{
		//최대 대기 시간
		ACQUIRE_TIMEOUT_TICK = 10000,

		//최대 대기 스핀 횟수
		MAX_SPIN_COUNT = 5000,

		//상위 16비트만 뽑아오기 위한 비트마스크
		WRITE_THREAD_MASK = 0xFFFF0000,

		//하위 16비트만 뽑아오기 위한 비트마스크
		READ_COUNT_MASK = 0x0000FFFF,

		EMPTY_FLAG = 0x00000000
	};

public:
	Lock();
	~Lock();

	Lock(const Lock& _Other) = delete;
	Lock(Lock&& _Other) noexcept = delete;
	Lock& operator=(const Lock& _Other) = delete;
	Lock& operator=(const Lock&& _Other) noexcept = delete;

	void WriteLock(const char* name);
	void WriteUnlock(const char* name);
	void ReadLock(const char* name);
	void ReadUnlock(const char* name);

private:
	Atomic<uint32> _lockFlag = EMPTY_FLAG;

	/*
		동일한 스레드가 Lock을 재귀적으로 잡는 것을 허락하기 위해 따로 멤버변수로 두었음.
		만약 Lock을 잡고 또 잡았다면 크래시 내지 않고 _writeCount를 증가시킴
	*/
	uint16 _writeCount = 0;
};


/*----------------------------------------------------
						Lock Guard
----------------------------------------------------*/

class ReadLockGuard
{
public:
	ReadLockGuard(Lock& lock, const char* name)
		:_lock(lock)
		, _name(name)
	{
		_lock.ReadLock(_name);
	}

	~ReadLockGuard()
	{
		_lock.ReadUnlock(_name);
	}

private:
	Lock& _lock;
	const char* _name;
};

class WriteLockGuard
{
public:
	WriteLockGuard(Lock& lock, const char* name)
		:_lock(lock)
		,_name(name)
	{
		_lock.WriteLock(_name);
	}

	~WriteLockGuard()
	{
		_lock.WriteUnlock(_name);
	}

private:
	Lock& _lock;
	const char* _name;
};