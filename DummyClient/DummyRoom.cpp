#include "pch.h"
#include "DummyRoom.h"
#include "ServerPacketHandler.h"

DummyRoomRef GRoom = MakeShared<DummyRoom>();

DummyRoom::DummyRoom()
{
	
}

DummyRoom::~DummyRoom()
{

}

void DummyRoom::Enter(DummyPlayerRef player)
{
	const uint64 id = player->id;
	if (_objects.end() != _objects.find(id))
		return;
	
	_objects[id] = player;
	DoTimer(200, &DummyRoom::Move, id);
}

void DummyRoom::Move(uint64 id)
{
	if (_objects.end() == _objects.find(id))
		return;

	DummyPlayerRef player = _objects[id];
	PacketSessionRef session = player->session.lock();
	if (nullptr == session)
		return;
	
	Protocol::PosInfo& nextPos = player->nowPos;
	nextPos.set_x(nextPos.x() + GetRandom(-100.f, 100.f));
	nextPos.set_y(nextPos.y() + GetRandom(-100.f, 100.f));
	
	Protocol::C_MOVE movePkt;
	movePkt.mutable_current_pos()->CopyFrom(nextPos);
	SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(movePkt);
	session->Send(sendBuffer);

	DoTimer(200, &DummyRoom::Move, id);
}
