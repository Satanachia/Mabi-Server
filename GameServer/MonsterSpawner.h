#pragma once

class Room;
class Object;
struct MonsterSpawnData;

class MonsterSpawner : public std::enable_shared_from_this<MonsterSpawner>
{
public:
	MonsterSpawner();
	~MonsterSpawner();
	
	MonsterSpawner(const MonsterSpawner& _Other) = delete;
	MonsterSpawner(MonsterSpawner&& _Other) noexcept = delete;
	MonsterSpawner& operator=(const MonsterSpawner& _Other) = delete;
	MonsterSpawner& operator=(const MonsterSpawner&& _Other) noexcept = delete;

public:
	//spawnDelay가 0인 경우 한번만 스폰
	void Init(OUT int32& cursor, const Vector<BYTE>& spawnDatas, Protocol::CreatureType spawnType, uint64 spawnDelay = 0);

	void SpawnAll(RoomRef room);
	void DeSpawnAll(RoomRef room);

	bool IsWalking() { return _isWorking; }
	
	const MonsterSpawnData& GetSpawnData(int32 spawnIndex);

private:
	void Spawn(RoomRef room, int32 spawnerIndex);
	void MonsterLeaveCallBack(ObjectRef object);

private:
	uint64 _spawnDelay = 0;
	
	bool _isWorking = false;
	Protocol::CreatureType _spawnType = Protocol::CREATURE_TYPE_NONE;
	Vector<MonsterSpawnData> _spawnDatas;
	HashMap<uint64, MonsterRef> _monsters;
};


/*-----------------------------------------------------------------------------
										MonsterSpawnData
-----------------------------------------------------------------------------*/

struct MonsterSpawnData
{
public:
	void ReadData(OUT int32& cursor, const Vector<BYTE>& datas);

	template <typename T>
	static T Read(OUT int32& cursor, const Vector<BYTE>& datas)
	{
		ASSERT_CRASH(cursor + sizeof(T) <= datas.size());

		T value = *reinterpret_cast<const T* const>(&datas[cursor]);
		cursor += sizeof(T);
		return value;
	}

private:
	Protocol::PosInfo ReadPosition(OUT int32& cursor, const Vector<BYTE>& datas);

public:
	Protocol::PosInfo spawnPos;
	Vector<Protocol::PosInfo> destinations;
};
