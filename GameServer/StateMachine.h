#pragma once
#include "Protocol.pb.h"

class StateMachine : public std::enable_shared_from_this<StateMachine>
{
	friend class Monster;

public:
	StateMachine();
	~StateMachine();
	
public:
	void ChangeState(Protocol::CreatureState key, bool isForce = false);
	
	Protocol::CreatureState GetCurState();

	void SetTickTime(uint64 tickTime) { _tickTime = tickTime; }

private:
	template <typename T>
	void CreateState(Protocol::CreatureState key)
	{
		ASSERT_CRASH(false == _allState.contains(key));

		MonsterRef monster = _monsterWeakPtr.lock();
		if (nullptr == monster)
			return;
		
		BaseStateRef newState = MakeShared<T>();
		newState->_fsm = shared_from_this();
		newState->_key = key;
		newState->monsterWeakPtr = monster;
		
		newState->Start();
		_allState[key] = newState;
	}
	
	void Update();

private:
	HashMap<Protocol::CreatureState, BaseStateRef> _allState;
	BaseStateRef _currentState = nullptr;
	uint64 _tickTime = 5000;

	std::weak_ptr<Monster> _monsterWeakPtr;
};
