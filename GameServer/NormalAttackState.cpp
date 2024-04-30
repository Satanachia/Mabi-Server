#include "pch.h"
#include "NormalAttackState.h"
#include "Monster.h"
#include "Room.h"


NormalAttackState::NormalAttackState()
{

}

NormalAttackState::~NormalAttackState()
{

}

void NormalAttackState::EnterState()
{
	BaseState::EnterState();
	//SendAnimationPacket(Protocol::ANI_TYPE_NORMAL_ATTACK);

	CreatureRef target = GetTarget();
	if (nullptr == target)
	{
		ChangeState(Protocol::CREATURE_STATE_IDLE);
		return;
	}
	
	//공격 애니메이션 브로드 캐스팅
	SendAnimationPacket(Protocol::ANI_TYPE_NORMAL_ATTACK);
	SendLookPacket(target);
	
	//실제 공격 처리
	MonsterRef monster = GetMonster();
	ASSERT_CRASH(monster);
	monster->AttackTarget(target, NORMAL_ATTACK_TARGET);
	
	enum { TICK_TIME = 2000 };
	SetTickTime(TICK_TIME);
	ReserveChangeState(TICK_TIME, Protocol::CREATURE_STATE_IDLE);
}

void NormalAttackState::SendLookPacket(CreatureRef target)
{
	MonsterRef monster = GetMonster();
	ASSERT_CRASH(monster);

	RoomRef room = monster->GetRoom();
	NULL_RETURN(room);

	Protocol::S_LOOK pkt;
	pkt.set_object_id(monster->GetObjectInfo().object_id());
	pkt.set_look_target(target->GetObjectInfo().object_id());
	SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
	room->Broadcast(sendBuffer);
}
