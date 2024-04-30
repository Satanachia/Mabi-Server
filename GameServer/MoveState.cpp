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

	//Ÿ���� �����Ѵٸ� Ÿ�ٹ������� �̵�
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

	//Ÿ���� �i�� �ִ� ��Ȳ�� �ƴҶ��� ȣ��Ǿ�� ��
	ASSERT_CRASH(false == _isTraceTarget);
	
	int32 spawnIndex = monster->GetSpawnerIndex();
	const MonsterSpawnData& spawnData = room->GetSpawnData(spawnIndex);
	if (true == spawnData.destinations.empty())
	{
		SetDirection(spawnData.spawnPos);
		return;
	}

	//���� ��ġ �� �ٸ������� �̵��ϴ� ���
	if (0 <= _destinationIndex)
	{
		_destinationIndex = -1;
		SetDirection(spawnData.spawnPos);
		return;
	}

	//���� ��ġ�� �̵��ϴ� ���
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
	//Ÿ���� ������
	_dir = GetTargetDirection();
	
	//Ÿ���� ��ģ ���
	if (true == UtilMath::IsZero(_dir))
	{
		_isTraceTarget = false;
		SetDestination();
		Move();
		return;
	}
	
	//Ÿ�ٰ��� �Ÿ� ���ϱ�
	float length = UtilMath::Length(_dir);
	
	//Ÿ�ٰ� ������ �ִٸ� Idle���·� ����
	if (length < Monster::hitAreaRange)
	{
		ChangeState(Protocol::CREATURE_STATE_IDLE);
		return;
	}
	
	//���⸸ �����
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

