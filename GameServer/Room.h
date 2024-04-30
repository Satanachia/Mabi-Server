#pragma once
#include "JobQueue.h"
#include <filesystem>
#include "MonsterSpawner.h"
#include "Protocol.pb.h"

struct MonsterSpawnData;

/*----------------------------
						Room
----------------------------*/

class Room : public JobQueue
{
	friend class RoomManager;
	
public:
	Room(Protocol::LevelType type);
	virtual ~Room();

	Room(const Room& _Other) = delete;
	Room(Room&& _Other) noexcept = delete;
	Room& operator=(const Room& _Other) = delete;
	Room& operator=(const Room&& _Other) noexcept = delete;

	RoomRef GetRoomRef() { return std::static_pointer_cast<Room>(shared_from_this()); }
	
public:
	/*------------------------------------------------------
						����/����/�̵�/��Ŷ ����
	------------------------------------------------------*/

	bool EnterRoom(ObjectRef object, bool isEnterGame, Protocol::PosInfo spawnPos);
	bool LeaveRoom(ObjectRef object);
	
	void Broadcast(SendBufferRef sendBuffer, uint64 exceptId = 0);
	
public:
	/*------------------------------------------------------
						�浹
	------------------------------------------------------*/

	void CollisionCircle(const Protocol::PosInfo& attactPoint, float radius, OUT Vector<CreatureRef>& collisionObjects, uint64 ignoreId = 0);
		
private:
	void Collision(std::function<bool(const Protocol::PosInfo&, float)> collisionProc, OUT Vector<CreatureRef>& collisionObjects, uint64 ignoreId = 0);

public:
	/*------------------------------------------------------
						������
	------------------------------------------------------*/

	bool IsPlayerExist();
	ObjectRef FindObject(uint64 objId);
	Protocol::LevelType GetRoomType() { return _roomType; }
	const MonsterSpawnData& GetSpawnData(int32 spawnIndex);
	const String& GetName() { return _name; }

private:
	bool AddObject(ObjectRef object);
	bool RemoveObject(uint64 objectId);

	//�濡 ������ ����� �÷��̾��� ��� �濡 ���� ����� ���� �÷��̾�� �˸���.
	void ApproveEnterGame(PlayerRef player, bool success);
	
	//�濡 ���������� �ٸ� �÷��̾�鿡�� ���� ���� ����� �˸���
	void BroadcastEnterNewB(ObjectRef object);
	
	//�濡 ������ ����� �÷��̾��� ��� ������ ������ �÷��̾���� ���񿡰� �˷��ش�
	void IntroduceOldB(PlayerRef player);

private:
	HashMap<uint64, ObjectRef> _objects;
	Protocol::LevelType _roomType = Protocol::LEVEL_TYPE_NONE;
	MonsterSpawnerRef _spawner = nullptr;
	String _name;
};


/*----------------------------
				RoomManager
----------------------------*/


class RoomManager
{
public:
	RoomManager();
	~RoomManager();
	
	RoomRef GetRoom(Protocol::LevelType roomType) { return rooms[roomType]; }
	
private:
	void LoadSpawnData(const wchar_t* FileName);

	template <typename T>
	T Read(OUT int32& cursor, const Vector<BYTE>& datas)
	{
		ASSERT_CRASH(cursor + sizeof(T) <= datas.size());

		T value = *reinterpret_cast<const T* const>(&datas[cursor]);
		cursor += sizeof(T);
		return value;
	}

private:
	RoomRef rooms[Protocol::LEVEL_TYPE_COUNT];
	static std::filesystem::path dataPath;
};

extern RoomManager GRoomManager;