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
	
	//������ ���� ������ ���
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

	//�濡�� �i�ܳ����ٸ� �۾��� ���� ����
	RoomRef room = monster->GetRoom();
	if (nullptr == room)
		return;
	
	//�ֿ� FSM ���� ����
	_currentState->Update();
	
	//_tickTime(�и�������) ���Ŀ� �ٽ� ����ɱ�
	room->DoTimer(_tickTime, [monster, this]()
	{
		//ĸó�� MonsterRef�� �־
		//���� ������ ������ Tick������ ���� ��

		//�濡�� �i�ܳ��� �ʾҴٸ� ����
		RoomRef room = monster->GetRoom();
		if (nullptr == room)
			return;
		
		//monster�� ����ִٸ� this�� ����ִ�
		//Update ���(?) ȣ��
		this->Update();
	});
}

Protocol::CreatureState StateMachine::GetCurState()
{
	return _currentState->_key;
}

