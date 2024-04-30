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
	//따로 스폰을 지정하지 않은 경우엔 스폰하지 않음
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
	//따로 스폰을 지정하지 않은 경우엔 스폰하지 않음
	if (Protocol::CREATURE_TYPE_NONE == _spawnType)
		return;

	if (false == _isWorking)
		return;
	
	//몬스터 생성
	MonsterRef monster = ObjectUtils::CreateCreature<Monster>(_spawnType, "Monster");
	
	//몬스터 스폰 위치
	int32 spawnPosIndex = Utility::GetRandom(0, static_cast<int>(_spawnDatas.size() - 1));
	const Protocol::PosInfo& spawnPos = _spawnDatas[spawnPosIndex].spawnPos;
	monster->GetCurrentPos().CopyFrom(spawnPos);
	
	//이 스포너의 몬스터 도감에 등록
	_monsters[monster->GetObjectInfo().object_id()] = monster;
	ASSERT_CRASH(0 <= spawnerIndex && spawnerIndex < _spawnDatas.size());
	monster->SetSpawnerIndex(spawnerIndex);
	ObjectRef monObj = monster;
	

	//몬스터 소멸시 스폰 예약하기(_spawnDelay이 0인경우 스폰 예약 안함)
	if (0 < _spawnDelay)
	{
		monObj->SetLeaveCallBack(std::bind(&MonsterSpawner::MonsterLeaveCallBack, shared_from_this(), std::placeholders::_1));
	}
	
	//방에 입장하고 나서 해야할 콜백 넘기기
	Protocol::CreatureType spawnType = _spawnType;
	monster->SetEnterCallBack([spawnType](ObjectRef object)
	{
		MonsterRef monster = std::static_pointer_cast<Monster>(object);
		monster->InitState(spawnType);	//FSM상태 만들기
		monster->StartUpdateTick();			//방에 입장했다면 Update루프 돌려주기
	});
	
	//방에 입장
	room->DoASync(&Room::EnterRoom, monObj, false, spawnPos);
}

void MonsterSpawner::MonsterLeaveCallBack(ObjectRef object)
{
	//해시맵에서 몬스터 지우기(인자가 값형이라 해시맵에서 지워도 refCnt 유지되어 있을꺼임. 아마도. 제발 ㅎ)
	uint64 id = object->GetObjectInfo().object_id();
	ASSERT_CRASH(_monsters.contains(id));
	_monsters.erase(id);
	
	
	//방안에 플레이어가 없는 등의 이유로 영업종료라면
	if (false == _isWorking)
		return;

	
	RoomRef room = object->GetRoom();
	if (nullptr == room)
		return;

	MonsterRef monster = std::static_pointer_cast<Monster>(object);
	int32 spawnerIndex = monster->GetSpawnerIndex();
	ASSERT_CRASH(0 <= spawnerIndex && spawnerIndex < _spawnDatas.size());
	MonsterSpawnerRef spawner = shared_from_this();
	
	//spawnDelay밀리세컨드 뒤에 몬스터 스폰요청
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
