#pragma once
#include "Protocol.pb.h"

class StateMachine;
class Monster;

class BaseState : public std::enable_shared_from_this<BaseState>
{
	friend class StateMachine;
	
public:
	BaseState()
	{
		
	}

protected:
	virtual void Start(){}
	virtual void EnterState();
	virtual void Update(){}
	virtual void ExitState() {}
	
protected:
	StateMachineRef GetFSM() { return _fsm.lock(); }
	void ChangeState(Protocol::CreatureState state);
	
	Protocol::CreatureState GetThisState() { return _key; }
	
	MonsterRef GetMonster() { return monsterWeakPtr.lock(); }

	void SetTickTime(uint64 tickTime);
	void ReserveChangeState(uint64 tickTime, Protocol::CreatureState nextState);

	Protocol::PosInfo GetTargetDirection(bool isNormalize = false);
	CreatureRef GetTarget();

	void SendAnimationPacket(Protocol::AnimationType _Type);

private:
	std::weak_ptr<StateMachine> _fsm;
	Protocol::CreatureState _key = Protocol::CREATURE_STATE_NONE;
	std::weak_ptr<Monster> monsterWeakPtr;
};
