#pragma once
#include "Creature.h"
#include "StateMachine.h"

class Monster : public Creature
{
public:
	static const float hitAreaRange;

	Monster();
	virtual ~Monster() override;

	Monster(const Monster& _Other) = delete;
	Monster(Monster&& _Other) noexcept = delete;
	Monster& operator=(const Monster& _Other) = delete;
	Monster& operator=(const Monster&& _Other) noexcept = delete;

	void SetSpawnerIndex(int32 spawnerIndex) { _spawnerIndex = spawnerIndex; }
	int32 GetSpawnerIndex() { return _spawnerIndex; }

	//FSM상태 만들어주기
	void InitState(Protocol::CreatureType _type);

	//Update 타이머 걸어주기
	void StartUpdateTick();
	
	void ChangeState(Protocol::CreatureState state, bool isForce = false);

	CreatureRef GetTarget();

protected:
	virtual void OnDamaged(CreatureRef attacker, AttackDamage damage, const Protocol::PosInfo& lookAt) override;

private:
	StateMachineRef _fsm;
	int32 _spawnerIndex = -1;
	std::weak_ptr<Creature> _target;
};

