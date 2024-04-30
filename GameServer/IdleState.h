#pragma once
#include "BaseState.h"

class IdleState : public BaseState
{
public:
	IdleState();
	~IdleState();

	IdleState(const IdleState& _Other) = delete;
	IdleState(IdleState&& _Other) noexcept = delete;
	IdleState& operator=(const IdleState& _Other) = delete;
	IdleState& operator=(const IdleState&& _Other) noexcept = delete;

	
protected:
	virtual void EnterState() override;

private:

};

