#pragma once

using TL = TypeList<class Object, class Creature, class Player, class Monster, class FieldItem, class Projectile>;

class Object : public std::enable_shared_from_this<Object>
{
public:
	Object();
	virtual ~Object();
	DECLARE_TL

	Object(const Object& _Other) = delete;
	Object(Object&& _Other) noexcept = delete;
	Object& operator=(const Object& _Other) = delete;
	Object& operator=(const Object&& _Other) noexcept = delete;

	void SetObjectID(uint64 id);
	void SetName(std::string_view name);
	void SetRoom(RoomRef room);
	RoomRef GetRoom() { return _room.load(); }
	void LeaveRoom(RoomRef room);

	bool IsPlayer() { return _isPlayer; }
	void SetEnterCallBack(std::function<void(ObjectRef)> callback) { _enterCallBack = callback; }
	void SetLeaveCallBack(std::function<void(ObjectRef)> callback) { _leaveCallBack = callback; }
	
	void Destroy(uint64 leaveTime = 0);
	bool IsDeath() { return !_isAlive; }

	Protocol::PosInfo& GetCurrentPos() { return _currentPos; }
	const Protocol::ObjectInfo& GetObjectInfo() { return _objectInfo; }
	void SetObjectInfoState(Protocol::CreatureState state) { _objectInfo.set_state(state); }
	const String& GetName() { return _name; }

protected:
	Protocol::ObjectInfo& GetObjectInfoRef() { return _objectInfo; }

protected:
	bool _isPlayer = false;

private:
	Protocol::ObjectInfo _objectInfo;
	Protocol::PosInfo _currentPos;
	std::function<void(ObjectRef)> _enterCallBack = nullptr;
	std::function<void(ObjectRef)> _leaveCallBack = nullptr;

	//스마트 포인터를 Set할때는
	//실제 포인터를 변경 + refCount를 변경하기 때문에
	//원자적이지 않다 -> 멀티 스레드에 취약할 수 있다. 그래서 아토믹을 이용
	Atomic<RoomRef> _room;

	bool _isAlive = true;
	String _name;
};

