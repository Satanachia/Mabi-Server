#include "pch.h"
#include "MoveState.h"

#include "Monster.h"
#include "Utility.h"
#include "Room.h"
#include "UtilMath.h"


MoveState::MoveState()
{
	
}

MoveState::~MoveState()
{
	
}

enum
{
	TICK_TIME = 200,
};

void MoveState::EnterState()
{
	BaseState::EnterState();

	SetTickTime(TICK_TIME);

	//타겟이 존재한다면 타겟방향으로 이동
	_dir = GetTargetDirection(true);
	if (false == UtilMath::IsZero(_dir))
	{
		_isTraceTarget = true;
		return;
	}

	SetDestination();
}


void MoveState::SetDestination()
{
	MonsterRef monster = GetMonster();
	NULL_RETURN(monster);

	RoomRef room = monster->GetRoom();
	NULL_RETURN(room);

	//타겟을 쫒고 있는 상황이 아닐때만 호출되어야 함
	ASSERT_CRASH(false == _isTraceTarget);
	
	int32 spawnIndex = monster->GetSpawnerIndex();
	const MonsterSpawnData& spawnData = room->GetSpawnData(spawnIndex);
	if (true == spawnData.destinations.empty())
	{
		SetDirection(spawnData.spawnPos);
		return;
	}

	//스폰 위치 외 다른곳으로 이동하던 경우
	if (0 <= _destinationIndex)
	{
		_destinationIndex = -1;
		SetDirection(spawnData.spawnPos);
		return;
	}

	//스폰 위치로 이동하던 경우
	_destinationIndex = Utility::GetRandom<int32>(0, static_cast<int32>(spawnData.destinations.size()) - 1);
	SetDirection(spawnData.destinations[_destinationIndex]);
}

void MoveState::SetDirection(const Protocol::PosInfo& destination)
{
	_destination = destination;

	MonsterRef monster = GetMonster();
	NULL_RETURN(monster);

	Protocol::PosInfo start = monster->GetCurrentPos();
	_dir = UtilMath::Minus(destination, start);
	_dir = UtilMath::Normalize(_dir);
}


void MoveState::Update()
{
	BaseState::Update();
	
	if (true == _isTraceTarget)
	{
		MoveToTarget();
	}
	else
	{
		Move();
	}
}

void MoveState::MoveToTarget()
{
	//타겟의 방향계산
	_dir = GetTargetDirection();
	
	//타겟을 놓친 경우
	if (true == UtilMath::IsZero(_dir))
	{
		_isTraceTarget = false;
		SetDestination();
		Move();
		return;
	}
	
	//타겟과의 거리 구하기
	float length = UtilMath::Length(_dir);
	
	//타겟과 가까이 있다면 Idle상태로 변경
	if (length < Monster::hitAreaRange)
	{
		ChangeState(Protocol::CREATURE_STATE_IDLE);
		return;
	}
	
	//방향만 남기기
	_dir = UtilMath::Normalize(_dir);
	Move();
}

void MoveState::Move()
{
	MonsterRef monster = GetMonster();
	NULL_RETURN(monster);

	Protocol::PosInfo nowPos = monster->GetCurrentPos();
	nowPos = UtilMath::Plus(nowPos, UtilMath::Multi(_dir, _speed), true);
	monster->GetCurrentPos().CopyFrom(nowPos);
	SendMovePacket();

	float distance = UtilMath::Length(UtilMath::Minus(_destination, nowPos));
	if (Monster::hitAreaRange < distance)
		return;

	ChangeState(Protocol::CREATURE_STATE_IDLE);
}




void MoveState::SendMovePacket(bool isLastMove /*= false*/)
{
	MonsterRef monster = GetMonster();
	NULL_RETURN(monster);

	RoomRef room = monster->GetRoom();
	NULL_RETURN(room);

	const Protocol::ObjectInfo& objInfo = monster->GetObjectInfo();
	
	Protocol::S_MOVE movePkt;
	movePkt.set_object_id(objInfo.object_id());
	Protocol::PosInfo monCurPos = monster->GetCurrentPos();
	monCurPos.set_z(0.f);
	movePkt.mutable_current_pos()->CopyFrom(monCurPos);
	if (true == isLastMove)
	{
		//movePkt.set_state(Protocol::CREATURE_STATE_IDLE);
	}
	
	SendBufferRef buffer = ClientPacketHandler::MakeSendBuffer(movePkt);
	room->Broadcast(buffer);
}




void MoveState::ExitState()
{
	BaseState::ExitState();
	SendMovePacket(true);
	_isTraceTarget = false;
}

