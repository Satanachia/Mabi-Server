#include "pch.h"
#include "Monster.h"
#include "Room.h"
#include "ObjectUtils.h"
#include "FieldItem.h"

#include "IdleState.h"
#include "MoveState.h"
#include "HitReactState.h"
#include "NormalAttackState.h"

const float Monster::hitAreaRange = 200.f;

Monster::Monster()
{
	INIT_TL(Monster);
	GConsoleLogger->ReserveMsg(Color::YELLOW, L"Monster\n");
}

Monster::~Monster()
{
	_fsm = nullptr;
	GConsoleLogger->ReserveMsg(Color::YELLOW, L"~Monster\n");
}

void Monster::InitState(Protocol::CreatureType _type)
{
	_fsm = MakeShared<StateMachine>();
	_fsm->_monsterWeakPtr = std::static_pointer_cast<Monster>(shared_from_this());
	
	_fsm->CreateState<IdleState>(Protocol::CREATURE_STATE_IDLE);
	_fsm->CreateState<MoveState>(Protocol::CREATURE_STATE_MOVE);
	_fsm->CreateState<NormalAttackState>(Protocol::CREATURE_STATE_NORMAR_ATTACK);
	_fsm->CreateState<HitReactState>(Protocol::CREATURE_STATE_HIT_REACT);
	
	_fsm->ChangeState(Protocol::CREATURE_STATE_IDLE);
}

void Monster::StartUpdateTick()
{
	RoomRef room = GetRoom();
	
	//입장과 동시에 방에서 거의 쫒겨난경우? 가 있을까?
	ASSERT_CRASH(room);
		
	room->DoASync([this]() {_fsm->Update(); });
}

void Monster::ChangeState(Protocol::CreatureState state, bool isForce /*= false*/)
{
	_fsm->ChangeState(state, isForce);
}

CreatureRef Monster::GetTarget()
{
	if (true == _target.expired())
	{
		_target = std::weak_ptr<Creature>();
		return nullptr;
	}

	CreatureRef target = _target.lock();
	if (nullptr == target)
	{
		_target = std::weak_ptr<Creature>();
		return nullptr;
	}

	//같은 방에 있는지 확인
	if (GetRoom() != target->GetRoom())
	{
		_target = std::weak_ptr<Creature>();
		return nullptr;
	}
	
	return target;
}


void Monster::OnDamaged(CreatureRef attacker, AttackDamage damage, const Protocol::PosInfo& lookAt)
{
	Creature::OnDamaged(attacker, damage, lookAt);
	
	_target = attacker;
	_fsm->ChangeState(Protocol::CREATURE_STATE_HIT_REACT, true);
	
	int32 hp = GetObjectInfo().hp();
	if (0 < hp)
		return;
	
	if (true == IsDeath())
		return;
	
	Destroy(3000);
	DropItem();
}
