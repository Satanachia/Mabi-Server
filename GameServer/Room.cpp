#include "pch.h"
#include "Room.h"
#include "Player.h"
#include "GameSession.h"
#include "ObjectUtils.h"
#include "Object.h"
#include "Creature.h"
#include "Monster.h"
#include "FileUtils.h"

RoomManager GRoomManager;
std::filesystem::path RoomManager::dataPath;

/*----------------------------
						Room
----------------------------*/

Room::Room(Protocol::LevelType type)
	:_roomType(type)
{
	
}

Room::~Room()
{
	
}

/*------------------------------------------------------
						����/����/�̵�/��Ŷ ����
------------------------------------------------------*/

bool Room::EnterRoom(ObjectRef object, bool isEnterGame, Protocol::PosInfo spawnPos)
{
	bool success = AddObject(object);
	Protocol::PosInfo& objSpawnPos = object->GetCurrentPos();
	objSpawnPos.CopyFrom(spawnPos);

	PlayerRef player = TypeCast<Player>(object);
	//�÷��̾ ���ӿ� ó�� ������ ��� �濡 ���� ����� ���� �÷��̾�� �˸���
	if (nullptr != player && true == isEnterGame)
	{
		ApproveEnterGame(player, success);
	}
	
	//�÷��̾ �����ߴµ��� ���� �����ʰ� ���� ���ϴ� ���̿��ٸ� �� ����
	if (nullptr != player && false == _spawner->IsWalking())
	{
		_spawner->SpawnAll(GetRoomRef());
	}

	
	//�ٸ� �÷��̾�鿡�� ���� ���� ����� �˸���
	BroadcastEnterNewB(object);

	//������Ʈ�� �÷��̾��� ��� ������ ������ �÷��̾���� ���񿡰� �˷��ش�
	if(nullptr != player)
	{
		IntroduceOldB(player);
	}

	return true;
}

void Room::ApproveEnterGame(PlayerRef player, bool success)
{
	Protocol::S_ENTER_GAME enterGamePkt;
	enterGamePkt.set_success(success);

	Protocol::ObjectInfo* playerInfo = new Protocol::ObjectInfo;
	playerInfo->CopyFrom(player->GetObjectInfo());
	enterGamePkt.set_allocated_player(playerInfo);

	GameSessionRef session = player->GetSession();
	if (nullptr == session)
		return;
	
	SEND_PACKERT(enterGamePkt);
}

void Room::BroadcastEnterNewB(ObjectRef object)
{
	Protocol::S_SPAWN spawnPkt;

	Protocol::ObjectInfo* objectInfo = spawnPkt.add_players();
	Protocol::PosInfo* posInfo = spawnPkt.add_positions();
	objectInfo->CopyFrom(object->GetObjectInfo());
	posInfo->CopyFrom(object->GetCurrentPos());
	spawnPkt.set_level(_roomType);
	
	SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(spawnPkt);
	Broadcast(sendBuffer, object->GetObjectInfo().object_id());
}

void Room::IntroduceOldB(PlayerRef player)
{
	Protocol::S_SPAWN spawnPkt;

	//��� �����ߴ� ����� ����
	uint64 id = player->GetObjectInfo().object_id();
	for (auto& pair : _objects)
	{
		/*if (false == pair.second->IsPlayer())
			continue;*/
		
		if (id == pair.second->GetObjectInfo().object_id())
			continue;

		Protocol::ObjectInfo* playerInfo = spawnPkt.add_players();
		Protocol::PosInfo* posInfo = spawnPkt.add_positions();

		playerInfo->CopyFrom(pair.second->GetObjectInfo());
		posInfo->CopyFrom(pair.second->GetCurrentPos());
	}
	spawnPkt.set_level(_roomType);

	GameSessionRef session = player->GetSession();
	if (nullptr == session)
		return;
	
	SEND_PACKERT(spawnPkt);
}





bool Room::LeaveRoom(ObjectRef object)
{
	if (nullptr == object)
		return false;

	const uint64 objectId = object->GetObjectInfo().object_id();

	//�� �������� _object���� �������� ��ü�� �����Ѵ�
	bool success = RemoveObject(objectId);

	//���� ����� �ٸ� �÷��̾�鿡�� �˸���
	Protocol::S_DESPAWN despawnPkt;
	despawnPkt.add_object_ids(objectId);

	//Broadcast�� ���ڷ� objectId�� ������ ������ �Ȱ���
	//LeavePlayer�� ȣ���� �������� �̹� _players���� �������� �÷��̾�� �������� �ʴ´�
	SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(despawnPkt);
	Broadcast(sendBuffer, objectId);
	
	//�Ʒ����ʹ� �÷��̾��� ��쿡��
	PlayerRef player = TypeCast<Player>(object);
	if (nullptr == player)
		return true;

	if (false == IsPlayerExist())
	{
		//���������� ���� �÷��̾��� ��� �濡 �ִ� ���͵��� �����Ѵ�
		_spawner->DeSpawnAll(GetRoomRef());
	}
	
	GameSessionRef session = player->GetSession();
	if (nullptr != session)
	{
		SEND_PACKERT(despawnPkt);
	}
	
	return true;
}



bool Room::AddObject(ObjectRef object)
{
	//�ߺ� üũ
	uint64 id = object->GetObjectInfo().object_id();
	if (_objects.end() != _objects.find(id))
		return false;

	_objects[id] = object;
	object->SetRoom(GetRoomRef());
	GConsoleLogger->ReserveMsg(Color::GREEN, L"[Enter Room(%s)]\nName(%d) : %s\n", _name.c_str(), id, object->GetName().c_str());
	return true;
}

bool Room::RemoveObject(uint64 objectId)
{
	static HashMap<uint64, String> names;

	//���� ��쿣 ������ ����
	if (_objects.end() == _objects.find(objectId))
		return false;
	
	ObjectRef object = _objects[objectId];
	object->LeaveRoom(GetRoomRef());
	_objects.erase(objectId);
	names[objectId] = object->GetName();
	GConsoleLogger->ReserveMsg(Color::GREEN, L"[Leave Room(%s)]\nName(%d) : %s\n", _name.c_str(), objectId, names[objectId].c_str());
	return true;
}

void Room::Broadcast(SendBufferRef sendBuffer, uint64 exceptId)
{
	for (const std::pair<uint64, ObjectRef>& pair : _objects)
	{
		ObjectRef object = pair.second;
		PlayerRef player = TypeCast<Player>(object);
		if (nullptr == player)
			continue;

		if (exceptId == player->GetObjectInfo().object_id())
			continue;

		GameSessionRef session = player->GetSession();
		if (nullptr != session)
			session->Send(sendBuffer);
	}
}

bool Room::IsPlayerExist()
{
	for (const std::pair<uint64, ObjectRef>& pair : _objects)
	{
		ObjectRef obj = pair.second;
		if (nullptr == obj)
			continue;

		if (true == obj->IsPlayer())
			return true;
	}
	
	return false;
}

ObjectRef Room::FindObject(uint64 objId)
{
	if (false == _objects.contains(objId))
		return nullptr;

	return _objects[objId];
}

const MonsterSpawnData& Room::GetSpawnData(int32 spawnIndex)
{
	return _spawner->GetSpawnData(spawnIndex);
}


/*------------------------------------------------------
						�浹
------------------------------------------------------*/

void Room::CollisionCircle(const Protocol::PosInfo& attactPoint, float radius, OUT Vector<CreatureRef>& collisionObjects, uint64 ignoreId /*= 0*/)
{
	Collision([&attactPoint, radius](const Protocol::PosInfo& objPos, float objRadius) -> bool
	{
		float distance = UtilMath::Length(UtilMath::Minus(attactPoint, objPos));
		return distance < (radius + objRadius);
	}
	, collisionObjects, ignoreId);
}

void Room::Collision(std::function<bool(const Protocol::PosInfo&, float)> collisionProc, OUT Vector<CreatureRef>& collisionObjects, uint64 ignoreId /*= 0*/)
{
	collisionObjects.clear();
	collisionObjects.reserve(_objects.size());
	
	for (const std::pair<uint64, ObjectRef>& pair : _objects)
	{
		if (ignoreId == pair.first)
			continue;

		CreatureRef creature = TypeCast<Creature>(pair.second);
		if (nullptr == creature)
			continue;

		//�ϴ� �ӽ�
		if (nullptr == TypeCast<Monster>(creature))
			continue;

		if (true == creature->IsDeath())
			continue;
		
		const Protocol::PosInfo& creaturePos = creature->GetCurrentPos();
		float radius = creature->GetCollisionRadius();
		if (false == collisionProc(creaturePos, radius))
			continue;
		
		collisionObjects.push_back(creature);
	}
}


/*----------------------------
				RoomManager
----------------------------*/

RoomManager::RoomManager() 
{
	//����� Ǯ���� ���� ���� �ֵ��� std::make_shared

	for (int32 i = Protocol::LEVEL_TYPE_NONE + 1; i < Protocol::LEVEL_TYPE_COUNT; ++i)
	{
		rooms[i] = std::make_shared<Room>(static_cast<Protocol::LevelType>(i)); 
		rooms[i]->_spawner = std::make_shared<MonsterSpawner>();
	}
	
	LoadSpawnData(L"OutDoorLevel");
	LoadSpawnData(L"CastleFrontYardLevel");
}

RoomManager::~RoomManager()
{
	for (int32 i = Protocol::LEVEL_TYPE_NONE + 1; i < Protocol::LEVEL_TYPE_COUNT; ++i)
	{
		rooms[i] = nullptr;
	}
}

void RoomManager::LoadSpawnData(const wchar_t* FileName)
{
	if (true == dataPath.empty())
	{
		dataPath = std::filesystem::current_path();
		dataPath = dataPath.parent_path();
		dataPath += L"\\Common\\MonsterSpawnPositions\\";
		ASSERT_CRASH(std::filesystem::exists(dataPath));
	}

	Vector<BYTE> spawnDatas;
	std::filesystem::path path = dataPath.wstring() + FileName;
	FileUtils::ReadFile(path, OUT spawnDatas);

	int32 cursor = 0;
	int32 levelTypeInt = MonsterSpawnData::Read<int32>(OUT cursor, spawnDatas);
	Protocol::LevelType levelType = static_cast<Protocol::LevelType>(levelTypeInt);
	ASSERT_CRASH(Protocol::LEVEL_TYPE_NONE < levelType && levelType < Protocol::LEVEL_TYPE_COUNT);
	rooms[levelType]->_name = FileName;
	rooms[levelType]->_spawner->Init(cursor, spawnDatas, Protocol::CREATURE_TYPE_MONSTER_IGGY, 5000);
}
