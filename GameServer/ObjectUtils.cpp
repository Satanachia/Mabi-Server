#include "pch.h"
#include "ObjectUtils.h"
#include "Player.h"
#include "GameSession.h"
#include "Projectile.h"
#include "FieldItem.h"

PlayerRef ObjectUtils::CreatePlayer(GameSessionRef session, Protocol::CreatureType  job, std::string_view name)
{
	PlayerRef player = CreateCreature<Player>(job, name);
	player->SetSession(session);	//weak_ptr
	session->SetPlayer(player);
	return player;
}

ProjectileRef ObjectUtils::CreateProjectile(Protocol::AttackType attackType)
{
	ProjectileRef obj = CreateObject<Projectile>(Protocol::OBJECT_TYPE_PROJECTILE, static_cast<uint64>(attackType), "Projectile");
	return obj;
}

FieldItemRef ObjectUtils::CreateItem(Protocol::ItemType itemType, int32 count /*= 1*/)
{
	FieldItemRef obj = CreateObject<FieldItem>(Protocol::OBJECT_TYPE_ITEM, static_cast<uint64>(itemType), "Item");
	obj->Init(count);
	return obj;
}

Protocol::ObjectType ObjectUtils::FindObjectType(uint64 objectId)
{
	const uint64 objTypeBitArea = static_cast<uint64>(FULL_BLOCK_BIT) << FIRST_BIT_BLOCK_POS;
	uint64 objType = (objectId & objTypeBitArea);
	objType >>= FIRST_BIT_BLOCK_POS;
	return static_cast<Protocol::ObjectType>(objType);
}

uint64 ObjectUtils::FindSecondType(uint64 objectId)
{
	const uint64 secondBitArea = static_cast<uint64>(FULL_BLOCK_BIT) << SECOND_BIT_BLOCK_POS;
	uint64 type = (objectId & secondBitArea);
	type >>= SECOND_BIT_BLOCK_POS;
	return type;
}
