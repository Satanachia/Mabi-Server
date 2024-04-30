#pragma once
#include "Protocol.pb.h"

/*
	오브젝트 생성과 관련된 부분은 이 곳에서 모아서 처리
*/
class ObjectUtils
{
	enum
	{
		FULL_BLOCK_BIT = 0xF,
		FIRST_BIT_BLOCK_POS = 60,
		SECOND_BIT_BLOCK_POS = 56
	};

public:
	static PlayerRef CreatePlayer(GameSessionRef session, Protocol::CreatureType job, std::string_view name);
	static ProjectileRef CreateProjectile(Protocol::AttackType attackType);
	static FieldItemRef CreateItem(Protocol::ItemType itemType, int32 count = 1);

	template <typename T>
	static std::shared_ptr<T> CreateObject(Protocol::ObjectType objType, uint64 secondType, std::string_view name)
	{
		static Atomic<uint64> s_idGenerator = 1;
		uint64 newId = s_idGenerator.fetch_add(1);

		const uint64 objTypeBit = static_cast<uint64>(objType) << FIRST_BIT_BLOCK_POS;
		newId |= objTypeBit;
		const uint64 secondTypeBit = secondType << SECOND_BIT_BLOCK_POS;
		newId |= secondTypeBit;

		std::shared_ptr<T> obj = MakeShared<T>();
		obj->SetObjectID(newId);
		obj->SetName(name);
		return obj;
	}

	template <typename T>
	static std::shared_ptr<T> CreateCreature(Protocol::CreatureType creatureType, std::string_view name)
	{
		std::shared_ptr<T> obj = CreateObject<T>(Protocol::OBJECT_TYPE_CREATURE, static_cast<uint64>(creatureType), name);
		return obj;
	}


	
	

	static Protocol::ObjectType FindObjectType(uint64 objectId);
	static uint64 FindSecondType(uint64 objectId);
		
public:
	
private:
	

};

