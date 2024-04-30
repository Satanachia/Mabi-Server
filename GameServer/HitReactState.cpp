#include "pch.h"
#include "HitReactState.h"
#include "Monster.h"
#include "Room.h"

HitReactState::HitReactState()
{

}

HitReactState::~HitReactState()
{
	
}

void HitReactState::EnterState()
{
	BaseState::EnterState();
	++continueCount;
	
	enum { CHANGE_TIME = 2000 };
	SetTickTime(CHANGE_TIME);
	
	MonsterRef monster = GetMonster();
	if (nullptr == monster)
		return;
	
	RoomRef room = GetMonster()->GetRoom();
	if (nullptr == room)
		return;
	
	std::weak_ptr<Monster> monWeak = GetWeak(monster);
	room->DoTimer(CHANGE_TIME, std::bind(&HitReactState::ChangeStateCallback, this, monWeak));
}



void HitReactState::ChangeStateCallback(std::weak_ptr<Monster> monWeak)
{
	MonsterRef monster = monWeak.lock();
	if (nullptr == monster)
		return;

	if (true == monster->IsDeath())
		return;

	//���Ͱ� ��������� This�� ��ȿ��
	--continueCount;
	
	//HitReactState ���¿��� �� ���ݹ��� ���
	//(roomŸ�̸ӿ� ChangeStateCallback ������ ���� ��������)
	if (0 < continueCount)
		return;
	
	Protocol::PosInfo dirToTarget = GetTargetDirection();
	if (true == UtilMath::IsZero(dirToTarget))
	{
		//Ÿ���� ��ȿ���� �ʴٸ� 
		ChangeState(Protocol::CREATURE_STATE_IDLE);
		return;
	}
	
	float distance = UtilMath::Length(dirToTarget);
	if (distance < Monster::hitAreaRange)
	{
		ChangeState(Protocol::CREATURE_STATE_NORMAR_ATTACK);
	}
	else
	{
		ChangeState(Protocol::CREATURE_STATE_MOVE);
	}
}


void HitReactState::ExitState()
{
	BaseState::ExitState();
	continueCount = 0;
}