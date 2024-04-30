#include "pch.h"
#include "StateMachine.h"
#include "BaseState.h"
#include "Room.h"
#include "Monster.h"

StateMachine::StateMachine()
{
	
}

StateMachine::~StateMachine()
{
	
}

void StateMachine::ChangeState(Protocol::CreatureState key, bool isForce /*= false*/)
{
	BaseStateRef nextState = _allState[key];
	ASSERT_CRASH(nextState);
	
	//이전과 같은 상태인 경우
	if ((nextState == _currentState) && (false == isForce))
		return;

	if (nullptr != _currentState)
	{
		_currentState->ExitState();
	}
	
	_currentState = nextState;
	nextState->EnterState();
}

void StateMachine::Update()
{
	if (nullptr == _currentState)
		return;
	
	MonsterRef monster = _monsterWeakPtr.lock();
	if (nullptr == monster)
		return;

	//방에서 쫒겨났었다면 작업을 하지 않음
	RoomRef room = monster->GetRoom();
	if (nullptr == room)
		return;
	
	//주요 FSM 로직 실행
	_currentState->Update();
	
	//_tickTime(밀리세컨드) 이후에 다시 예약걸기
	room->DoTimer(_tickTime, [monster, this]()
	{
		//캡처에 MonsterRef를 넣어서
		//갈땐 가더라도 마지막 Tick까지는 돌고 감

		//방에서 쫒겨나지 않았다면 실행
		RoomRef room = monster->GetRoom();
		if (nullptr == room)
			return;
		
		//monster가 살아있다면 this도 살아있다
		//Update 재귀(?) 호출
		this->Update();
	});
}

Protocol::CreatureState StateMachine::GetCurState()
{
	return _currentState->_key;
}

