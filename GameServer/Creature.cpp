#include "pch.h"
#include <algorithm>

#include "Creature.h"
#include "Room.h"
#include "Monster.h"
#include "Player.h"
#include "ObjectUtils.h"
#include "FieldItem.h"

Creature::Creature()
{
	INIT_TL(Creature);
	//GetObjectInfo().set_object_type(Protocol::OBJECT_TYPE_CREATURE);
}

Creature::~Creature()
{

}

bool Creature::CanAttack(CreatureRef target)
{
	ObjectRef thisObj = shared_from_this();
	if (nullptr != TypeCast<Player>(thisObj) && nullptr != TypeCast<Monster>(target))
		return true;

	if (nullptr != TypeCast<Monster>(thisObj) && nullptr != TypeCast<Player>(target))
		return true;
	
	return false;
}

void Creature::AttackTarget(CreatureRef target, AttackDamage damage)
{
	CreatureRef ThisPtr = TypeCast<Creature>(shared_from_this());
	ASSERT_CRASH(ThisPtr);

	if (false == CanAttack(target))
		return;

	target->OnDamaged(ThisPtr, damage, GetCurrentPos());
}

void Creature::AttackTarget(uint64 targetId, AttackDamage damage)
{
	RoomRef room = GetRoom();
	NULL_RETURN(room);

	CreatureRef target = TypeCast<Creature>(room->FindObject(targetId));
	NULL_RETURN(target);

	AttackTarget(target, damage);
}




Protocol::PosInfo Creature::GetLookDirection(uint64 targetId, bool isNormalize /*= false*/)
{
	RoomRef room = GetRoom();
	NULL_RETURN(room) Protocol::PosInfo();
	
	ObjectRef target = room->FindObject(targetId);
	NULL_RETURN(target) Protocol::PosInfo();
	
	if(true == target->IsDeath())
		return Protocol::PosInfo();

	return GetLookDirection(target, isNormalize);
}

Protocol::PosInfo Creature::GetLookDirection(const Protocol::PosInfo& targetPos, bool isNormalize /*= false*/)
{
	const Protocol::PosInfo& ThisPos = GetCurrentPos();
	Protocol::PosInfo dirResult = UtilMath::Minus(targetPos, ThisPos);

	if (true == isNormalize)
	{
		dirResult = UtilMath::Normalize(dirResult);
	}
	
	return dirResult;
}

void Creature::OnDamaged(CreatureRef attacker, AttackDamage damage, const Protocol::PosInfo& lookAt)
{
	RoomRef room = GetRoom();
	NULL_RETURN(room);

	Protocol::ObjectInfo& objInfo = GetObjectInfoRef();
	uint64 id = objInfo.object_id();

	//Hp 처리
	int32 hp = objInfo.hp();
	hp -= damage;
	if (hp <= 0)
	{
		hp = 0;
	}
	objInfo.set_hp(hp);

	Protocol::S_ANIMATION aniPkt;
	aniPkt.set_object_id(id);
	Protocol::AnimationType aniType = (0 == hp) ? Protocol::ANI_TYPE_DEATH : Protocol::ANI_TYPE_DAMAGED;
	aniPkt.set_ani_type(aniType);
	SendBufferRef aniSendBuffer = ClientPacketHandler::MakeSendBuffer(aniPkt);
	room->Broadcast(aniSendBuffer);

	//회전
	Protocol::S_LOOK lookPkt;
	lookPkt.set_object_id(id);
	lookPkt.mutable_look_pos()->CopyFrom(lookAt);
	SendBufferRef lookSendBuffer = ClientPacketHandler::MakeSendBuffer(lookPkt);
	room->Broadcast(lookSendBuffer);

	//피격
	Protocol::S_ATTACK attackPkt;
	attackPkt.set_victim_id(id);
	attackPkt.set_hp(hp);
	SendBufferRef attackSendBuffer = ClientPacketHandler::MakeSendBuffer(attackPkt);
	room->Broadcast(attackSendBuffer);
}

void Creature::DropItem(Protocol::ItemType type /*= Protocol::ITEM_TYPE_NONE*/, int32 count /*= 1*/)
{
	ASSERT_CRASH(0 < count);
	if (Protocol::ITEM_TYPE_NONE == type)
	{
		int32 randomItem = Utility::GetRandom(Protocol::ITEM_TYPE_NONE + 1, Protocol::ITEM_TYPE_COUNT - 1);
		type = static_cast<Protocol::ItemType>(randomItem);
		ASSERT_CRASH(Protocol::ITEM_TYPE_NONE < type);
		ASSERT_CRASH(type < Protocol::ITEM_TYPE_COUNT);
	}
	
	RoomRef room = GetRoom();
	NULL_RETURN(room);
	
	ObjectRef item = ObjectUtils::CreateItem(type, count);
	const Protocol::PosInfo& itemSpawnPos = GetCurrentPos();
	room->EnterRoom(item, false, itemSpawnPos);
}


void Creature::SendParticlePacket(const Protocol::PosInfo& attackPos, Protocol::ParticleType particleType)
{
	const Protocol::PosInfo& CurPos = GetCurrentPos();
	Protocol::PosInfo dir = UtilMath::Minus(attackPos, CurPos);
	float yaw = UtilMath::GetAngleDegree(dir);
	
	SendParticlePacket(attackPos, yaw, particleType);
}

void Creature::SendParticlePacket(float yaw, Protocol::ParticleType particleType)
{
	SendParticlePacket(GetCurrentPos(), yaw, particleType);
}

void Creature::SendParticlePacket(const Protocol::PosInfo& spawnPos, float yaw, Protocol::ParticleType particleType)
{
	RoomRef room = GetRoom();
	NULL_RETURN(room);
	
	Protocol::S_SPAWN_PARTICLE particlePkt;
	particlePkt.set_level(room->GetRoomType());
	particlePkt.mutable_position()->CopyFrom(spawnPos);
	particlePkt.set_yaw(yaw);
	particlePkt.set_particle_type(particleType);
	
	SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(particlePkt);
	room->Broadcast(sendBuffer);
}
