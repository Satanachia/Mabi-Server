#pragma once

/*----------------------------------------------------
						Read&Write Spin Lock
----------------------------------------------------*/

/*
	Read&Write Lock�ʿ伺
		������ ���� ������ �����ʹ� Lock�� ���� �ʿ� ����
		�׷��� ���� Ư���ϰ� ������ �ʿ��� ��찡 ����� ����
		�� Ư���� ��� ������ ��ҿ��� Lock�� ��°� �ʹ� �Ʊ��
		�̷��� ����� �� �ִ� Lock�� Read&Write 
*/

/*
	�� 32��Ʈ�� �����
	���� 16��Ʈ�� Write�� �Ҷ� �� ���� ���� �������� ID�� �ǹ���
	���� 16��Ʈ�� Read�Ҷ� ���� ������� �������� ������ �ǹ���
*/

class Lock
{
	enum : uint32
	{
		//�ִ� ��� �ð�
		ACQUIRE_TIMEOUT_TICK = 10000,

		//�ִ� ��� ���� Ƚ��
		MAX_SPIN_COUNT = 5000,

		//���� 16��Ʈ�� �̾ƿ��� ���� ��Ʈ����ũ
		WRITE_THREAD_MASK = 0xFFFF0000,

		//���� 16��Ʈ�� �̾ƿ��� ���� ��Ʈ����ũ
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
		������ �����尡 Lock�� ��������� ��� ���� ����ϱ� ���� ���� ��������� �ξ���.
		���� Lock�� ��� �� ��Ҵٸ� ũ���� ���� �ʰ� _writeCount�� ������Ŵ
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