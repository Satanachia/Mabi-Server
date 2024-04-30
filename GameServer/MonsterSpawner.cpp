#include "pch.h"
#include "MonsterSpawner.h"
#include "ObjectUtils.h"
#include "Room.h"
#include "Monster.h"
#include "Utility.h"
#include "StateMachine.h"

MonsterSpawner::MonsterSpawner()
{
}

MonsterSpawner::~MonsterSpawner()
{
	
}


void MonsterSpawner::Init(OUT int32& cursor, const Vector<BYTE>& spawnDatas, Protocol::CreatureType spawnType, uint64 spawnDelay /*= 0*/)
{
	_spawnType = spawnType;
	_spawnDelay = spawnDelay;

	int32 spawnerCount = MonsterSpawnData::Read<int32>(OUT cursor, spawnDatas);
	_spawnDatas.resize(spawnerCount);
	for (int32 i = 0; i < spawnerCount; ++i)
	{
		_spawnDatas[i].ReadData(OUT cursor, spawnDatas);
	}
}

void MonsterSpawner::SpawnAll(RoomRef room)
{
	//���� ������ �������� ���� ��쿣 �������� ����
	if (Protocol::CREATURE_TYPE_NONE == _spawnType)
		return;
	
	if (true == _isWorking)
		return;
	
	_isWorking = true;
	for (int32 i = 0; i < _spawnDatas.size(); ++i)
	{
		Spawn(room, i);
	}
}


const MonsterSpawnData& MonsterSpawner::GetSpawnData(int32 spawnIndex)
{
	ASSERT_CRASH(0 <= spawnIndex && spawnIndex < _spawnDatas.size());
	return _spawnDatas[spawnIndex];
}

void MonsterSpawner::Spawn(RoomRef room, int32 spawnerIndex)
{
	//���� ������ �������� ���� ��쿣 �������� ����
	if (Protocol::CREATURE_TYPE_NONE == _spawnType)
		return;

	if (false == _isWorking)
		return;
	
	//���� ����
	MonsterRef monster = ObjectUtils::CreateCreature<Monster>(_spawnType, "Monster");
	
	//���� ���� ��ġ
	int32 spawnPosIndex = Utility::GetRandom(0, static_cast<int>(_spawnDatas.size() - 1));
	const Protocol::PosInfo& spawnPos = _spawnDatas[spawnPosIndex].spawnPos;
	monster->GetCurrentPos().CopyFrom(spawnPos);
	
	//�� �������� ���� ������ ���
	_monsters[monster->GetObjectInfo().object_id()] = monster;
	ASSERT_CRASH(0 <= spawnerIndex && spawnerIndex < _spawnDatas.size());
	monster->SetSpawnerIndex(spawnerIndex);
	ObjectRef monObj = monster;
	

	//���� �Ҹ�� ���� �����ϱ�(_spawnDelay�� 0�ΰ�� ���� ���� ����)
	if (0 < _spawnDelay)
	{
		monObj->SetLeaveCallBack(std::bind(&MonsterSpawner::MonsterLeaveCallBack, shared_from_this(), std::placeholders::_1));
	}
	
	//�濡 �����ϰ� ���� �ؾ��� �ݹ� �ѱ��
	Protocol::CreatureType spawnType = _spawnType;
	monster->SetEnterCallBack([spawnType](ObjectRef object)
	{
		MonsterRef monster = std::static_pointer_cast<Monster>(object);
		monster->InitState(spawnType);	//FSM���� �����
		monster->StartUpdateTick();			//�濡 �����ߴٸ� Update���� �����ֱ�
	});
	
	//�濡 ����
	room->DoASync(&Room::EnterRoom, monObj, false, spawnPos);
}

void MonsterSpawner::MonsterLeaveCallBack(ObjectRef object)
{
	//�ؽøʿ��� ���� �����(���ڰ� �����̶� �ؽøʿ��� ������ refCnt �����Ǿ� ��������. �Ƹ���. ���� ��)
	uint64 id = object->GetObjectInfo().object_id();
	ASSERT_CRASH(_monsters.contains(id));
	_monsters.erase(id);
	
	
	//��ȿ� �÷��̾ ���� ���� ������ ����������
	if (false == _isWorking)
		return;

	
	RoomRef room = object->GetRoom();
	if (nullptr == room)
		return;

	MonsterRef monster = std::static_pointer_cast<Monster>(object);
	int32 spawnerIndex = monster->GetSpawnerIndex();
	ASSERT_CRASH(0 <= spawnerIndex && spawnerIndex < _spawnDatas.size());
	MonsterSpawnerRef spawner = shared_from_this();
	
	//spawnDelay�и������� �ڿ� ���� ������û
	room->DoTimer(_spawnDelay, [spawner, room, spawnerIndex]()
	{
		spawner->Spawn(room, spawnerIndex);
	});
}

void MonsterSpawner::DeSpawnAll(RoomRef room)
{
	_isWorking = false;
	
	for (const std::pair<uint64, MonsterRef>& pair : _monsters)
	{
		ObjectRef monster = pair.second;
		room->DoASync(&Room::LeaveRoom, monster);
	}
}


/*-----------------------------------------------------------------------------
										MonsterSpawnData
-----------------------------------------------------------------------------*/

void MonsterSpawnData::ReadData(OUT int32& cursor, const Vector<BYTE>& datas)
{
	spawnPos = ReadPosition(OUT cursor, datas);

	int32 destinationCount = Read<int32>(OUT cursor, datas);
	destinations.resize(destinationCount);
	for (Protocol::PosInfo& dest : destinations)
	{
		dest = ReadPosition(OUT cursor, datas);
	}
}

Protocol::PosInfo MonsterSpawnData::ReadPosition(OUT int32& cursor, const Vector<BYTE>& datas)
{
	Protocol::PosInfo result;

	float x = Read<float>(OUT cursor, datas);
	float y = Read<float>(OUT cursor, datas);
	float z = Read<float>(OUT cursor, datas);

	result.set_x(x);
	result.set_y(y);
	result.set_z(z);

	return result;
}
