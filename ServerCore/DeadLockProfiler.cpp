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

	//처음 발견된 노드인 경우 아이디 발급 및 등록
	if (_nameToId.end() == findIter)
	{
		lockKId = static_cast<int32>(_nameToId.size());
		_nameToId[name] = lockKId;
		_idToName[lockKId] = name;
	}

	//등록된 노드인 경우
	else
	{
		lockKId = findIter->second;
	}

	//잡고 있는 락이 있던 경우
	if (false == LlockStack.empty())
	{
		const int32 prevId = LlockStack.top();
		//재귀 상황이 아닌 경우
		if (lockKId != prevId)
		{
			std::set<int32>& history = _lockHistory[prevId];

			//새로 연결되는 노드인 경우
			if (history.end() == history.find(lockKId))
			{
				//노드를 추가하고
				history.insert(lockKId);
				//사이클 확인
				CheckCycle();
			}
		}

		//기존에 발견되지 않은 케이스라면
	}

	LlockStack.push(lockKId);
}

void DeadLockProfiler::PopLock(const char* name)
{
	LockGuard guard(_lock);

	//혹시 모르는 버그 체크1
	if (true == LlockStack.empty())
	{
		CRASH("MULTIPLE_UNLOCK");
	}

	//혹시 모르는 버그 체크2
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
	//이미 방문을 했었다면
	if (-1 != _discoveredOrder[now])
		return;

	//now노드의 방문 순서 기록
	_discoveredOrder[now] = _discoveredCount++;

	//인접한 노드 순회
	auto findIter = _lockHistory.find(now);

	//다른 락을 잡은 적이 없었다면
	if (_lockHistory.end() == findIter)
	{
		_finished[now] = true;
		return;
	}

	//다른 락을 잡은 적이 있다면
	std::set<int32>& nexts = findIter->second;
	for (int32 next : nexts)
	{
		//방문 한 적이 없다면
		if (-1 == _discoveredOrder[next])
		{
			_parent[next] = now;
			DFS(next);
			continue;
		}

		//이미 방문한 적이 있다
			//순방향 간선인지, 
			//교차간선인지
			//역방향 간선인지 확인

		//순방향 간선
		if (_discoveredOrder[now] < _discoveredOrder[next])
			continue;

		if (true == _finished[next])
			continue;

		//아직 DFS(next)가 종료되지 않았다
		//now는 next보다 숫자가 작다
		//역방향 간선이다
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


