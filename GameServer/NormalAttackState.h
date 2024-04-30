#pragma once
#include "BaseState.h"

class NormalAttackState : public BaseState
{
public:
	NormalAttackState();
	~NormalAttackState();

	NormalAttackState(const NormalAttackState& _Other) = delete;
	NormalAttackState(NormalAttackState&& _Other) noexcept = delete;
	NormalAttackState& operator=(const NormalAttackState& _Other) = delete;
	NormalAttackState& operator=(const NormalAttackState&& _Other) noexcept = delete;

protected:
	virtual void EnterState() override;

private:
	void SendLookPacket(CreatureRef target);
};

