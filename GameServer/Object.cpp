#include "pch.h"
#include "Object.h"
#include "Room.h"

Object::Object()
{
	INIT_TL(Object);

	enum { START_HP = 100 };
	_objectInfo.set_hp(START_HP);
}

Object::~Object()
{

}

void Object::SetObjectID(uint64 id)
{
	ASSERT_CRASH(0 == _objectInfo.object_id());
	_objectInfo.set_object_id(id);
}

void Object::SetName(std::string_view name)
{
	std::string mbcsName = std::string(name);
	_objectInfo.set_object_name(mbcsName);
	_name = Utility::AnsiToUniCode(mbcsName);
}

void Object::SetRoom(RoomRef room)
{
	_room = room;
	
	if (nullptr != _enterCallBack)
	{
		_enterCallBack(shared_from_this());
		_enterCallBack = nullptr;
	}
}

void Object::LeaveRoom(RoomRef room)
{
	if (nullptr != _leaveCallBack)
	{
		_leaveCallBack(shared_from_this());
		_leaveCallBack = nullptr;
	}
	
	//다른 스레드에서 이미 Room이 교체된 경우엔 _room = nullptr 처리하지 않음
	_room.compare_exchange_strong(room, nullptr);
}

void Object::Destroy(uint64 leaveTime /*= 0*/)
{
	if (false == _isAlive)
		return;
	
	_isAlive = false;
	RoomRef room = GetRoom();
	if (nullptr == room)
		return;
	
	if (0 == leaveTime)
	{
		room->DoASync(&Room::LeaveRoom, shared_from_this());
	}
	else
	{
		room->DoTimer(leaveTime, &Room::LeaveRoom, shared_from_this());
	}
}
