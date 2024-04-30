#pragma once
#include "Object.h"

enum AttackDamage
{
	NORMAL_ATTACK_NONE = 0,
	NORMAL_ATTACK_TARGET = 30,
	SKILL_ATTACK_MAGE = 80,
	SKILL_ATTACK_ROG = 50,
	SKILL_ATTACK_WARRIOR = 70,
};


class Creature : public Object
{
public:
	Creature();
	virtual ~Creature() override;

	Creature(const Creature& _Other) = delete;
	Creature(Creature&& _Other) noexcept = delete;
	Creature& operator=(const Creature& _Other) = delete;
	Creature& operator=(const Creature&& _Other) noexcept = delete;
	
	bool CanAttack(CreatureRef target);
	void AttackTarget(CreatureRef target, AttackDamage damage);
	void AttackTarget(uint64 targetId, AttackDamage damage);
	
	Protocol::PosInfo GetLookDirection(ObjectRef target, bool isNormalize = false) { return GetLookDirection(target->GetCurrentPos(), isNormalize); }
	Protocol::PosInfo GetLookDirection(uint64 targetId, bool isNormalize = false);
	Protocol::PosInfo GetLookDirection(const Protocol::PosInfo& targetPos, bool isNormalize = false);
	
	void SendParticlePacket(const Protocol::PosInfo& attackPos, Protocol::ParticleType particleType);
	void SendParticlePacket(float yaw, Protocol::ParticleType particleType);
	void SendParticlePacket(const Protocol::PosInfo& spawnPos, float yaw, Protocol::ParticleType particleType);
	
	float GetCollisionRadius() { return collisionRadius; }

	virtual void OnDamaged(CreatureRef attacker, AttackDamage damage, const Protocol::PosInfo& lookAt);

protected:
	void DropItem(Protocol::ItemType type = Protocol::ITEM_TYPE_NONE, int32 count = 1);

private:
	float collisionRadius = 34.f;
};

