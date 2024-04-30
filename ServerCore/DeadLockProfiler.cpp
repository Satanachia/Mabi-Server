#include "pch.h"
#include "DeadLockProfiler.h"

/*----------------------------------------------
								DeadLockProfiler
----------------------------------------------*/

DeadLockProfiler::DeadLockProfiler()
{

}

DeadLockProfiler::~DeadLockProfiler()
{

}

void DeadLockProfiler::PushLock(const char* name) 
{
	LockGuard guard(_lock);

	int32 lockKId = 0;
	auto findIter = _nameToId.find(name);

	//ó�� �߰ߵ� ����� ��� ���̵� �߱� �� ���
	if (_nameToId.end() == findIter)
	{
		lockKId = static_cast<int32>(_nameToId.size());
		_nameToId[name] = lockKId;
		_idToName[lockKId] = name;
	}

	//��ϵ� ����� ���
	else
	{
		lockKId = findIter->second;
	}

	//��� �ִ� ���� �ִ� ���
	if (false == LlockStack.empty())
	{
		const int32 prevId = LlockStack.top();
		//��� ��Ȳ�� �ƴ� ���
		if (lockKId != prevId)
		{
			std::set<int32>& history = _lockHistory[prevId];

			//���� ����Ǵ� ����� ���
			if (history.end() == history.find(lockKId))
			{
				//��带 �߰��ϰ�
				history.insert(lockKId);
				//����Ŭ Ȯ��
				CheckCycle();
			}
		}

		//������ �߰ߵ��� ���� ���̽����
	}

	LlockStack.push(lockKId);
}

void DeadLockProfiler::PopLock(const char* name)
{
	LockGuard guard(_lock);

	//Ȥ�� �𸣴� ���� üũ1
	if (true == LlockStack.empty())
	{
		CRASH("MULTIPLE_UNLOCK");
	}

	//Ȥ�� �𸣴� ���� üũ2
	int32 lockId = _nameToId[name];
	if (LlockStack.top() != lockId)
	{
		CRASH("INVALID_UNLOCK");
	}

	LlockStack.pop();
}

void DeadLockProfiler::CheckCycle()
{
	const int32 lockCount = static_cast<int32>(_nameToId.size());
	_discoveredOrder = std::vector<int32>(lockCount, -1);
	_discoveredCount = 0;
	_finished = std::vector<bool>(lockCount, false);
	_parent = std::vector<int32>(lockCount, -1);

	for (int32 lockId = 0; lockId < lockCount; ++lockId)
	{
		DFS(lockId);
	}

	_discoveredOrder.clear();
	_finished.clear();
	_parent.clear();
}

void DeadLockProfiler::DFS(int32 now)
{
	//�̹� �湮�� �߾��ٸ�
	if (-1 != _discoveredOrder[now])
		return;

	//now����� �湮 ���� ���
	_discoveredOrder[now] = _discoveredCount++;

	//������ ��� ��ȸ
	auto findIter = _lockHistory.find(now);

	//�ٸ� ���� ���� ���� �����ٸ�
	if (_lockHistory.end() == findIter)
	{
		_finished[now] = true;
		return;
	}

	//�ٸ� ���� ���� ���� �ִٸ�
	std::set<int32>& nexts = findIter->second;
	for (int32 next : nexts)
	{
		//�湮 �� ���� ���ٸ�
		if (-1 == _discoveredOrder[next])
		{
			_parent[next] = now;
			DFS(next);
			continue;
		}

		//�̹� �湮�� ���� �ִ�
			//������ ��������, 
			//������������
			//������ �������� Ȯ��

		//������ ����
		if (_discoveredOrder[now] < _discoveredOrder[next])
			continue;

		if (true == _finished[next])
			continue;

		//���� DFS(next)�� ������� �ʾҴ�
		//now�� next���� ���ڰ� �۴�
		//������ �����̴�
		printf("%s -> %s\n", _idToName[now], _idToName[next]);

		int32 parent = now;
		while (true)
		{
			printf("%s -> %s\n", _idToName[parent], _idToName[parent]);
			parent = _parent[parent];
			if (parent == next)
				break;
		}

		CRASH("DEADLOCK_DETECTED");
	}

	_finished[now] = true;
}


