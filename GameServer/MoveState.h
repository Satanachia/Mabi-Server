#pragma once
#include "BaseState.h"

class MoveState : public BaseState
{
public:
	MoveState();
	~MoveState();

	MoveState(const MoveState& _Other) = delete;
	MoveState(MoveState&& _Other) noexcept = delete;
	MoveState& operator=(const MoveState& _Other) = delete;
	MoveState& operator=(const MoveState&& _Other) noexcept = delete;

protected:
	virtual void EnterState() override;
	virtual void Update() override;
	virtual void ExitState() override;

private:
	void MoveToTarget();
	void Move();
	
	void SetDestination();
	void SetDirection(const Protocol::PosInfo& destination);
	void SendMovePacket(bool isLastMove = false);
	

private:
	Protocol::PosInfo _dir;
	const float _speed = 50.f;
	bool _isTraceTarget = false;
	
	int32 _destinationIndex = -1;
	Protocol::PosInfo _destination;
};

