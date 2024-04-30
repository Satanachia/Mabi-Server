#include "pch.h"
#include "IdleState.h"

#include "Monster.h"
#include "Utility.h"
#include "Room.h"
#include "StateMachine.h"

IdleState::IdleState()
{
	
}

IdleState::~IdleState()
{

}

void IdleState::EnterState()
{
	BaseState::EnterState();
	enum { MOVE_CHANGE_TIME = 5000 };
	SetTickTime(MOVE_CHANGE_TIME);

	Protocol::PosInfo dirToTarget = GetTargetDirection();
	if (true == UtilMath::IsZero(dirToTarget))
	{
		//타겟이 유효하지 않다면 
		ReserveChangeState(MOVE_CHANGE_TIME, Protocol::CREATURE_STATE_MOVE);
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


