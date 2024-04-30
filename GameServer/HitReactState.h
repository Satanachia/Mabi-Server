#pragma once
#include "BaseState.h"

class HitReactState : public BaseState
{
public:
	HitReactState();
	~HitReactState();

	HitReactState(const HitReactState& _Other) = delete;
	HitReactState(HitReactState&& _Other) noexcept = delete;
	HitReactState& operator=(const HitReactState& _Other) = delete;
	HitReactState& operator=(const HitReactState&& _Other) noexcept = delete;

protected:
	virtual void EnterState() override;
	virtual void ExitState() override;

private:
	void ChangeStateCallback(std::weak_ptr<class Monster> monWeak);
	
private:
	int32 continueCount = 0;
};

