#pragma once
#include "Object.h"

class Projectile : public Object
{
public:
	Projectile();
	~Projectile();

	Projectile(const Projectile& _Other) = delete;
	Projectile(Projectile&& _Other) noexcept = delete;
	Projectile& operator=(const Projectile& _Other) = delete;
	Projectile& operator=(const Projectile&& _Other) noexcept = delete;

	void Init(const Protocol::PosInfo& dir, CreatureRef owner);
	void StartUpdateTick();

	void SetSpeed(float speed) { _speed = speed; }
	void Update();

private:
	void Move();
	void Collision();
	void SendMovePacket();
	
private:
	CreatureRef _owner = nullptr;
	Protocol::PosInfo _dir;
	float _speed = 200.f;
	float _radius = 100.f;
	uint64 _tickTime = 100;
	uint64 _lifeCount = 30;
};

