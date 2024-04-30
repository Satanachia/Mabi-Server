#include "pch.h"
#include "BaseState.h"
#include "Monster.h"
#include "Room.h"

void BaseState::EnterState()
{
	MonsterRef monster = GetMonster();
	NULL_RETURN(monster);
	monster->SetObjectInfoState(_key);
}

void BaseState::ChangeState(Protocol::CreatureState state)
{
	StateMachineRef fsm = GetFSM();
	if (nullptr == fsm)
		return;
	
	fsm->ChangeState(state);
}

void BaseState::SetTickTime(uint64 tickTime)
{
	StateMachineRef fsm = GetFSM();
	if (nullptr == fsm)
		return;

	fsm->SetTickTime(tickTime);
}

void BaseState::ReserveChangeState(uint64 tickTime, Protocol::CreatureState nextState)
{
	RoomRef room = GetMonster()->GetRoom();
	if (nullptr == room)
		return;

	Protocol::CreatureState StateWhenReserve = _key;
	std::weak_ptr<StateMachine> fsmWeak = _fsm;
	room->DoTimer(tickTime, [fsmWeak, nextState, StateWhenReserve]()
	{
		StateMachineRef fsm = fsmWeak.lock();
		if (nullptr == fsm)
			return;
		
		//예약하던 당시 상태랑 현재상태가 같지 않다면(상태 변화가 있었다면) 실행하지 않음
		if (StateWhenReserve != fsm->GetCurState())
			return;
		
		fsm->ChangeState(nextState);
	});
}

Protocol::PosInfo BaseState::GetTargetDirection(bool isNormalize /*= false*/)
{
	MonsterRef monster = monsterWeakPtr.lock();
	if (nullptr == monster)
		return Protocol::PosInfo();

	CreatureRef target = monster->GetTarget();
	if (nullptr == target)
		return Protocol::PosInfo();
	
	return monster->GetLookDirection(target, isNormalize);
}

CreatureRef BaseState::GetTarget()
{
	MonsterRef monster = monsterWeakPtr.lock();
	if (nullptr == monster)
		return nullptr;

	return monster->GetTarget();
}

void BaseState::SendAnimationPacket(Protocol::AnimationType _Type)
{
	MonsterRef monster = GetMonster();
	NULL_RETURN(monster);
	
	Protocol::S_ANIMATION aniPkt;
	aniPkt.set_ani_type(_Type);
	aniPkt.set_object_id(monster->GetObjectInfo().object_id());
	SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(aniPkt);
	
	RoomRef room = monster->GetRoom();
	NULL_RETURN(room);
	room->Broadcast(sendBuffer);
}

