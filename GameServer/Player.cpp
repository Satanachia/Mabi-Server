#include "pch.h"
#include "Player.h"
#include "Room.h"
#include "GameSession.h"
#include "ObjectUtils.h"
#include "Projectile.h"
#include "DbManager.h"
#include "ConsoleLog.h"
#include "FieldItem.h"
#include "Object.h"
#include "GenProcedures.h"

Player::Player()
{
	INIT_TL(Player);
	_isPlayer = true;
	GConsoleLogger->ReserveMsg(Color::YELLOW, L"Player\n");
}

Player::~Player()
{
	GConsoleLogger->ReserveMsg(Color::YELLOW, L"~Player\n");
}


void Player::FetchInventory()
{
	PlayerRef thisPlayer = std::static_pointer_cast<Player>(shared_from_this());
	GDbManager->ReqQuery(std::bind(&DbManager::FetchItems, GDbManager, std::placeholders::_1, thisPlayer));
	//GDbManager->ReqQuery(std::bind(&DbManager::FetchMoney, GDbManager, std::placeholders::_1, thisPlayer));
}

void Player::AcquireItem(Protocol::ItemType itemType, int32 count /*= 1*/, bool addFromDB /*= false*/)
{
	ASSERT_CRASH(itemType != Protocol::ITEM_TYPE_NONE);
	ASSERT_CRASH(itemType != Protocol::ITEM_TYPE_COUNT);

	if (false == _items.contains(itemType))
	{
		_items[itemType] = ItemData(addFromDB);
	}
	
	ASSERT_CRASH(0 <= count);
	_items[itemType].count += count;
}

void Player::SendInitInventoryPacket()
{
	Protocol::S_INIT_INVENTORY pkt;
	for (const std::pair<Protocol::ItemType, ItemData>& item : _items)
	{
		Protocol::ItemType itemType = item.first;
		int32 itemCount = item.second.count;
		
		ASSERT_CRASH(itemType != Protocol::ITEM_TYPE_NONE);
		ASSERT_CRASH(itemType != Protocol::ITEM_TYPE_COUNT);
		if (itemCount <= 0)
			continue;
		
		Protocol::ItemInfo* itemInfo = pkt.add_items();
		itemInfo->set_type(itemType);
		itemInfo->set_count(itemCount);
	}
	
	GameSessionRef session = GetSession();
	ASSERT_CRASH(session);
	SEND_PACKERT(pkt);
}


void Player::HandleMove(const Protocol::C_MOVE& pkt)
{
	RoomRef room = GetRoom();
	if (nullptr == room)
		return;
	
	Protocol::ObjectInfo& objInfo = GetObjectInfoRef();
	uint64 objID = objInfo.object_id();

	Protocol::PosInfo& prevPos = GetCurrentPos();
	const Protocol::PosInfo& nextPos = pkt.current_pos();
	
	GConsoleLogger->ReserveMsg(Color::WHITE,
		L"[Player_Move]\n%s(%d) : (%.2f, %.2f, %.2f) -> (%.2f, %.2f, %.2f) Level : %s\n",
		GetName().c_str(), objID, prevPos.x(), prevPos.y(), prevPos.z(), nextPos.x(), nextPos.y(), nextPos.z(), room->GetName().c_str());
	
	//위치값 수정
	prevPos.CopyFrom(nextPos);

	//클라로부터 수신받은 이동요청을 다른 애들에게도 전송하기
	Protocol::S_MOVE movePkt;
	movePkt.set_object_id(objID);
	movePkt.mutable_current_pos()->CopyFrom(nextPos);
	
	SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(movePkt);
	room->Broadcast(sendBuffer, objID);
}

void Player::HandleAttack(const Protocol::C_ATTACK& pkt)
{
	static const uint64 attackWaitTime = 100;
	const uint64 nowAttackTime = ::GetTickCount64();
	if ((nowAttackTime - prevAttackTime) < attackWaitTime)
		return;
	
	prevAttackTime = nowAttackTime;

	//다른 플레이어에게 이 플레이어가 공격했음을 알림
	
	Protocol::AttackType attackType = pkt.attack_type();
	switch (attackType)
	{
	case Protocol::ATTACK_TYPE_NORMAL_ATTACK:
		AttackTarget(pkt.target_id(), NORMAL_ATTACK_TARGET);
		break;
	case Protocol::ATTACK_TYPE_SKILL_MAGE:
		AttackSkillMage(pkt.target_pos());
		break;
	case Protocol::ATTACK_TYPE_SKILL_WARRIOR:
		AttackSkillWarrior(pkt.target_pos());
		break;
	case Protocol::ATTACK_TYPE_SKILL_ROG:
		break;
	}
}

void Player::AttackSkillMage(const Protocol::PosInfo& attackPos)
{
	SendParticlePacket(attackPos, Protocol::PARTICLE_TYPE_METEO);

	RoomRef room = GetRoom();
	NULL_RETURN(room);

	uint64 ignoreId = GetObjectInfo().object_id();
	const float attackRadius = 500.f;
	CreatureRef thisRef = std::static_pointer_cast<Creature>(shared_from_this());
	
	Vector<CreatureRef> collisionObjects;
	room->CollisionCircle(attackPos, attackRadius, collisionObjects, ignoreId);
	for (CreatureRef creature : collisionObjects)
	{
		creature->OnDamaged(thisRef, AttackDamage::SKILL_ATTACK_MAGE, attackPos);
	}
}

void Player::AttackSkillWarrior(const Protocol::PosInfo& attackPos)
{
	CreatureRef thisRef = std::static_pointer_cast<Creature>(shared_from_this());
	const Protocol::PosInfo& playerPos = GetCurrentPos();
	Protocol::PosInfo dir = UtilMath::Minus(attackPos, playerPos);
	dir = UtilMath::Normalize(dir);

	RoomRef room = GetRoom();
	NULL_RETURN(room);

	ProjectileRef projectile = ObjectUtils::CreateProjectile(Protocol::ATTACK_TYPE_SKILL_WARRIOR);
	projectile->Init(dir, thisRef);
	room->EnterRoom(projectile, false, GetCurrentPos());
	projectile->StartUpdateTick();
}

void Player::HandleAddItem(const Protocol::C_ADD_ITEM& pkt)
{
	RoomRef room = GetRoom();
	if (nullptr == room)
		return;
	
	ObjectRef fieldItemObj = room->FindObject(pkt.item_id());
	if (nullptr == fieldItemObj)
		return;
	
	FieldItemRef fieldItem = TypeCast<FieldItem>(fieldItemObj);
	ASSERT_CRASH(fieldItem);
	const Protocol::ItemInfo& fieldItemInfo = fieldItem->GetItemInfo();
	AcquireItem(fieldItemInfo.type(), fieldItemInfo.count());
	
	GameSessionRef session = GetSession();
	Protocol::S_ADD_ITEM addItemPkt;
	Protocol::ItemInfo* item = addItemPkt.add_items();
	item->CopyFrom(fieldItemInfo);
	SEND_PACKERT(addItemPkt);

	room->LeaveRoom(fieldItemObj);
}

void Player::HandleDropItem(const Protocol::C_DROP_ITEM& pkt)
{
	const Protocol::ItemInfo& item = pkt.item();
	Protocol::ItemType itemType = item.type();
	int32 dropCount = item.count();
	if (0 == dropCount)
		return;
	
	HashMap<Protocol::ItemType, ItemData>::iterator findIter = _items.find(itemType);
	if (_items.end() == findIter)
		return;

	int32 holdItemCount = findIter->second.count;
	if (holdItemCount < dropCount)
	{
		uint64 id = GetObjectInfo().object_id();
		GConsoleLogger->ReserveMsg(Color::RED, L"[ID : %d] (holdItemCount : %d) < (dropCount : %d)\n", id, holdItemCount, dropCount);
		dropCount = holdItemCount;
	}
	
	if (0 == dropCount)
		return;

	//아이템 감소
	holdItemCount -= dropCount;
	findIter->second.count = holdItemCount;
	DropItem(itemType, dropCount);
}

void Player::HandleUseItem(const Protocol::C_USE_ITEM& pkt)
{
	enum
	{
		HP_PORTION = 30,
		LARGE_HP_PORTION = 80,
		MAX_HP = 100
	};

	Protocol::ItemType itemType = pkt.item();
	if (false == _items.contains(itemType))
		return;

	int32& itemCount = _items[itemType].count;
	if (itemCount <= 0)
	{
		itemCount = 0;
		return;
	}
	--itemCount;

	
	Protocol::ObjectInfo& info = GetObjectInfoRef();
	int32 hp = info.hp();
	switch (itemType)
	{
	case Protocol::ITEM_TYPE_HP_PORTION:
		hp += HP_PORTION;
		break;
	case Protocol::ITEM_TYPE_LARGE_HP_PORTION:
		hp += LARGE_HP_PORTION;
		break;
	}
	
	if (MAX_HP < hp)
	{
		hp = MAX_HP;
	}
	info.set_hp(hp);

	Protocol::S_USE_ITEM itemPkt;
	itemPkt.set_hp(hp);
	itemPkt.set_object_id(info.object_id());
	SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(itemPkt);
	
	RoomRef room = GetRoom();
	NULL_RETURN(room);
	room->Broadcast(sendBuffer);
}


void Player::SaveItems()
{
	std::wstring name = Utility::AnsiToUniCode(GetObjectInfo().object_name());
	for (const std::pair<Protocol::ItemType, ItemData>& item : _items)
	{
		Protocol::ItemType itemType = item.first;
		int32 itemCount = item.second.count;
		bool fromDB = item.second.createFromDB;
		
		ASSERT_CRASH(itemType != Protocol::ITEM_TYPE_NONE);
		ASSERT_CRASH(itemType != Protocol::ITEM_TYPE_COUNT);
		ASSERT_CRASH(0 <= itemCount);
		
		if (true == fromDB)
		{
			GDbManager->ReqQuery(std::bind(&DbManager::SaveItem<SP::UpdateItem>, GDbManager, std::placeholders::_1, name, itemType, itemCount));
		}
		else
		{
			GDbManager->ReqQuery(std::bind(&DbManager::SaveItem<SP::InsertItem>, GDbManager, std::placeholders::_1, name, itemType, itemCount));
		}
	}
}


