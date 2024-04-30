#include "pch.h"
#include "ServerPacketHandler.h"
#include "DummyRoom.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
	//TODO : Log
	return false;
}

bool Handle_S_LOGIN(PacketSessionRef& session, Protocol::S_LOGIN& pkt) 
{
	ASSERT_CRASH(true == pkt.success());
	ASSERT_CRASH(0 < pkt.players_size());
	
	//어떤 캐릭터로 입장할 지 결정한 후 게임입장 버튼 선택
	Protocol::C_ENTER_GAME enterGamePkt;
	enterGamePkt.set_playerindex(0);
	SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(enterGamePkt);
	session->Send(sendBuffer);
	return true;
}

bool Handle_S_ENTER_GAME(PacketSessionRef& session, Protocol::S_ENTER_GAME& pkt)
{
	DummyPlayerRef player = MakeShared<DummyPlayer>();
	const Protocol::ObjectInfo& info = pkt.player();
	player->id = info.object_id();
	player->session = session;

	player->nowPos.set_x(442.f);
	player->nowPos.set_y(45000.f);
	player->nowPos.set_z(24634.f);
	GRoom->DoASync(&DummyRoom::Enter, player);
	return true;
}

bool Handle_S_LEAVE_GAME(PacketSessionRef& session, Protocol::S_LEAVE_GAME& pkt)
{
	return true;
}

bool Handle_S_SPAWN(PacketSessionRef& session, Protocol::S_SPAWN& pkt)
{
	return false;
}

bool Handle_S_DESPAWN(PacketSessionRef& session, Protocol::S_DESPAWN& pkt)
{
	return false;
}

bool Handle_S_MOVE(PacketSessionRef& session, Protocol::S_MOVE& pkt)
{
	return false;
}

bool Handle_S_LOOK(PacketSessionRef& session, Protocol::S_LOOK& pkt)
{
	return false;
}

bool Handle_S_ANIMATION(PacketSessionRef& session, Protocol::S_ANIMATION& pkt)
{
	return false;
}

bool Handle_S_ATTACK(PacketSessionRef& session, Protocol::S_ATTACK& pkt)
{
	return false;
}

bool Handle_S_THINK(PacketSessionRef& session, Protocol::S_THINK& pkt)
{
	return false;
}

bool Handle_S_THINK_DONE(PacketSessionRef& session, Protocol::S_THINK_DONE& pkt)
{
	return false;
}

bool Handle_S_SPAWN_PARTICLE(PacketSessionRef& session, Protocol::S_SPAWN_PARTICLE& pkt)
{
	return false;
}

bool Handle_S_INIT_INVENTORY(PacketSessionRef& session, Protocol::S_INIT_INVENTORY& pkt)
{
	return false;
}

bool Handle_S_ADD_ITEM(PacketSessionRef& session, Protocol::S_ADD_ITEM& pkt)
{
	return false;
}

bool Handle_S_USE_ITEM(PacketSessionRef& session, Protocol::S_USE_ITEM& pkt)
{
	return false;
}

bool Handle_S_PONG(PacketSessionRef& session, Protocol::S_PONG& pkt)
{
	return false;
}

bool Handle_S_CHAT(PacketSessionRef& session, Protocol::S_CHAT& pkt)
{
	//std::cout << pkt.msg() << std::endl;
	return true;
}
