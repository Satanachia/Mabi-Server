#pragma once

/*----------------------------------------------
								DeadLockProfiler
----------------------------------------------*/

class DeadLockProfiler
{
public:
	DeadLockProfiler();
	~DeadLockProfiler();

	DeadLockProfiler(const DeadLockProfiler& _Other) = delete;
	DeadLockProfiler(DeadLockProfiler&& _Other) noexcept = delete;
	DeadLockProfiler& operator=(const DeadLockProfiler& _Other) = delete;
	DeadLockProfiler& operator=(const DeadLockProfiler&& _Other) noexcept = delete;

	void PushLock(const char* name);
	void PopLock(const char* name);
	void CheckCycle();

private:
	void DFS(int32 now);

private:
	std::unordered_map<const char*, int32>	_nameToId;
	std::unordered_map<int32, const char*>	_idToName;
	std::map<int32, std::set<int32>>					_lockHistory;

	Mutex _lock;

private:
	//노드가 발견된 순서를 기록하는 배열
	std::vector<int32>		_discoveredOrder;
	//노드가 발견된 순서
	int32							_discoveredCount = 0;
	//Dfs(i)가 종료되었는지 여부
	std::vector<bool>		_finished;
	std::vector<int32>		_parent;
};

