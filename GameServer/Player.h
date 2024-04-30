#pragma once
#include "Creature.h"

class GameSession;
class Room;
using RepeatedItems = google::protobuf::RepeatedPtrField<Protocol::ItemInfo>;

class Player : public Creature
{
public:
	Player();
	virtual ~Player() override;

	Player(const Player& _Other) = delete;
	Player(Player&& _Other) noexcept = delete;
	Player& operator=(const Player& _Other) = delete;
	Player& operator=(const Player&& _Other) noexcept = delete;

	GameSessionRef GetSession() { return _session.lock(); }
	void SetSession(GameSessionRef session) { _session = session; }

	void FetchInventory();
	void AcquireItem(Protocol::ItemType itemType, int32 count = 1, bool addFromDB = false);
	void SendInitInventoryPacket();

	void HandleMove(const Protocol::C_MOVE& pkt);
	void HandleAttack(const Protocol::C_ATTACK& pkt);
	void HandleAddItem(const Protocol::C_ADD_ITEM& pkt);
	void HandleDropItem(const Protocol::C_DROP_ITEM& pkt);
	void HandleUseItem(const Protocol::C_USE_ITEM& pkt);


	void SaveItems();
	
private:
	void AttackSkillMage(const Protocol::PosInfo& attackPos);
	void AttackSkillWarrior(const Protocol::PosInfo& attackPos);


private:
	//세션이 Room과 다르게 처음 값을 설정해 준 이후로
	// 값이 변경될 일이 없기 때문에 Atomic이 안 붙지 않았다
	std::weak_ptr<GameSession> _session;

	
	struct ItemData
	{
		ItemData():count(0), createFromDB(false){}
		ItemData(bool fromDB):count(0), createFromDB(fromDB){}
		int32 count;
		bool createFromDB;
	};
	HashMap<Protocol::ItemType, ItemData> _items;

	uint64 prevAttackTime = 0;
};

