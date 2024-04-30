#pragma once
#include "Protocol.pb.h"

#if UE_BUILD_DEBUG + UE_BUILD_DEVELOPMENT + UE_BUILD_TEST + UE_BUILD_SHIPPING >= 1
#include "Mabinogi.h"
#endif

using PacketHandlerFunc = std::function<bool(PacketSessionRef&, BYTE*, int32)>;
extern PacketHandlerFunc GPacketHandler[UINT16_MAX];

enum : uint16
{
	PKT_C_LOGIN = 1000,
	PKT_S_LOGIN = 1001,
	PKT_C_CREATE_NEW_PLAYER = 1002,
	PKT_C_ENTER_GAME = 1003,
	PKT_S_ENTER_GAME = 1004,
	PKT_C_LEAVE_GAME = 1005,
	PKT_S_LEAVE_GAME = 1006,
	PKT_S_SPAWN = 1007,
	PKT_S_DESPAWN = 1008,
	PKT_C_CHANGE_LEVEL = 1009,
	PKT_C_MOVE = 1010,
	PKT_S_MOVE = 1011,
	PKT_C_LOOK = 1012,
	PKT_S_LOOK = 1013,
	PKT_C_ANIMATION = 1014,
	PKT_S_ANIMATION = 1015,
	PKT_C_ATTACK = 1016,
	PKT_S_ATTACK = 1017,
	PKT_C_THINK = 1018,
	PKT_S_THINK = 1019,
	PKT_S_THINK_DONE = 1020,
	PKT_C_SPAWN_PARTICLE = 1021,
	PKT_S_SPAWN_PARTICLE = 1022,
	PKT_S_INIT_INVENTORY = 1023,
	PKT_C_DROP_ITEM = 1024,
	PKT_C_ADD_ITEM = 1025,
	PKT_S_ADD_ITEM = 1026,
	PKT_C_USE_ITEM = 1027,
	PKT_S_USE_ITEM = 1028,
	PKT_C_PING = 1029,
	PKT_S_PONG = 1030,
	PKT_C_CHAT = 1031,
	PKT_S_CHAT = 1032,
};

// Custom Handlers
bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len);
bool Handle_S_LOGIN(PacketSessionRef& session, Protocol::S_LOGIN& pkt);
bool Handle_S_ENTER_GAME(PacketSessionRef& session, Protocol::S_ENTER_GAME& pkt);
bool Handle_S_LEAVE_GAME(PacketSessionRef& session, Protocol::S_LEAVE_GAME& pkt);
bool Handle_S_SPAWN(PacketSessionRef& session, Protocol::S_SPAWN& pkt);
bool Handle_S_DESPAWN(PacketSessionRef& session, Protocol::S_DESPAWN& pkt);
bool Handle_S_MOVE(PacketSessionRef& session, Protocol::S_MOVE& pkt);
bool Handle_S_LOOK(PacketSessionRef& session, Protocol::S_LOOK& pkt);
bool Handle_S_ANIMATION(PacketSessionRef& session, Protocol::S_ANIMATION& pkt);
bool Handle_S_ATTACK(PacketSessionRef& session, Protocol::S_ATTACK& pkt);
bool Handle_S_THINK(PacketSessionRef& session, Protocol::S_THINK& pkt);
bool Handle_S_THINK_DONE(PacketSessionRef& session, Protocol::S_THINK_DONE& pkt);
bool Handle_S_SPAWN_PARTICLE(PacketSessionRef& session, Protocol::S_SPAWN_PARTICLE& pkt);
bool Handle_S_INIT_INVENTORY(PacketSessionRef& session, Protocol::S_INIT_INVENTORY& pkt);
bool Handle_S_ADD_ITEM(PacketSessionRef& session, Protocol::S_ADD_ITEM& pkt);
bool Handle_S_USE_ITEM(PacketSessionRef& session, Protocol::S_USE_ITEM& pkt);
bool Handle_S_PONG(PacketSessionRef& session, Protocol::S_PONG& pkt);
bool Handle_S_CHAT(PacketSessionRef& session, Protocol::S_CHAT& pkt);

class ServerPacketHandler
{
public:
	static void Init()
	{
		for (int32 i = 0; i < UINT16_MAX; i++)
			GPacketHandler[i] = Handle_INVALID;
		GPacketHandler[PKT_S_LOGIN] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_LOGIN>(Handle_S_LOGIN, session, buffer, len); };
		GPacketHandler[PKT_S_ENTER_GAME] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_ENTER_GAME>(Handle_S_ENTER_GAME, session, buffer, len); };
		GPacketHandler[PKT_S_LEAVE_GAME] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_LEAVE_GAME>(Handle_S_LEAVE_GAME, session, buffer, len); };
		GPacketHandler[PKT_S_SPAWN] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_SPAWN>(Handle_S_SPAWN, session, buffer, len); };
		GPacketHandler[PKT_S_DESPAWN] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_DESPAWN>(Handle_S_DESPAWN, session, buffer, len); };
		GPacketHandler[PKT_S_MOVE] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_MOVE>(Handle_S_MOVE, session, buffer, len); };
		GPacketHandler[PKT_S_LOOK] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_LOOK>(Handle_S_LOOK, session, buffer, len); };
		GPacketHandler[PKT_S_ANIMATION] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_ANIMATION>(Handle_S_ANIMATION, session, buffer, len); };
		GPacketHandler[PKT_S_ATTACK] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_ATTACK>(Handle_S_ATTACK, session, buffer, len); };
		GPacketHandler[PKT_S_THINK] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_THINK>(Handle_S_THINK, session, buffer, len); };
		GPacketHandler[PKT_S_THINK_DONE] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_THINK_DONE>(Handle_S_THINK_DONE, session, buffer, len); };
		GPacketHandler[PKT_S_SPAWN_PARTICLE] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_SPAWN_PARTICLE>(Handle_S_SPAWN_PARTICLE, session, buffer, len); };
		GPacketHandler[PKT_S_INIT_INVENTORY] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_INIT_INVENTORY>(Handle_S_INIT_INVENTORY, session, buffer, len); };
		GPacketHandler[PKT_S_ADD_ITEM] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_ADD_ITEM>(Handle_S_ADD_ITEM, session, buffer, len); };
		GPacketHandler[PKT_S_USE_ITEM] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_USE_ITEM>(Handle_S_USE_ITEM, session, buffer, len); };
		GPacketHandler[PKT_S_PONG] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_PONG>(Handle_S_PONG, session, buffer, len); };
		GPacketHandler[PKT_S_CHAT] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_CHAT>(Handle_S_CHAT, session, buffer, len); };
	}

	static bool HandlePacket(PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[header->id](session, buffer, len);
	}
	static SendBufferRef MakeSendBuffer(Protocol::C_LOGIN& pkt) { return MakeSendBuffer(pkt, PKT_C_LOGIN); }
	static SendBufferRef MakeSendBuffer(Protocol::C_CREATE_NEW_PLAYER& pkt) { return MakeSendBuffer(pkt, PKT_C_CREATE_NEW_PLAYER); }
	static SendBufferRef MakeSendBuffer(Protocol::C_ENTER_GAME& pkt) { return MakeSendBuffer(pkt, PKT_C_ENTER_GAME); }
	static SendBufferRef MakeSendBuffer(Protocol::C_LEAVE_GAME& pkt) { return MakeSendBuffer(pkt, PKT_C_LEAVE_GAME); }
	static SendBufferRef MakeSendBuffer(Protocol::C_CHANGE_LEVEL& pkt) { return MakeSendBuffer(pkt, PKT_C_CHANGE_LEVEL); }
	static SendBufferRef MakeSendBuffer(Protocol::C_MOVE& pkt) { return MakeSendBuffer(pkt, PKT_C_MOVE); }
	static SendBufferRef MakeSendBuffer(Protocol::C_LOOK& pkt) { return MakeSendBuffer(pkt, PKT_C_LOOK); }
	static SendBufferRef MakeSendBuffer(Protocol::C_ANIMATION& pkt) { return MakeSendBuffer(pkt, PKT_C_ANIMATION); }
	static SendBufferRef MakeSendBuffer(Protocol::C_ATTACK& pkt) { return MakeSendBuffer(pkt, PKT_C_ATTACK); }
	static SendBufferRef MakeSendBuffer(Protocol::C_THINK& pkt) { return MakeSendBuffer(pkt, PKT_C_THINK); }
	static SendBufferRef MakeSendBuffer(Protocol::C_SPAWN_PARTICLE& pkt) { return MakeSendBuffer(pkt, PKT_C_SPAWN_PARTICLE); }
	static SendBufferRef MakeSendBuffer(Protocol::C_DROP_ITEM& pkt) { return MakeSendBuffer(pkt, PKT_C_DROP_ITEM); }
	static SendBufferRef MakeSendBuffer(Protocol::C_ADD_ITEM& pkt) { return MakeSendBuffer(pkt, PKT_C_ADD_ITEM); }
	static SendBufferRef MakeSendBuffer(Protocol::C_USE_ITEM& pkt) { return MakeSendBuffer(pkt, PKT_C_USE_ITEM); }
	static SendBufferRef MakeSendBuffer(Protocol::C_PING& pkt) { return MakeSendBuffer(pkt, PKT_C_PING); }
	static SendBufferRef MakeSendBuffer(Protocol::C_CHAT& pkt) { return MakeSendBuffer(pkt, PKT_C_CHAT); }

private:
	template<typename PacketType, typename ProcessFunc>
	static bool HandlePacket(ProcessFunc func, PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketType pkt;
		if (pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader)) == false)
			return false;

		return func(session, pkt);
	}

	template<typename T>
	static SendBufferRef MakeSendBuffer(T& pkt, uint16 pktId)
	{
		const uint16 dataSize = static_cast<uint16>(pkt.ByteSizeLong());
		const uint16 packetSize = dataSize + sizeof(PacketHeader);

#if UE_BUILD_DEBUG + UE_BUILD_DEVELOPMENT + UE_BUILD_TEST + UE_BUILD_SHIPPING >= 1
		SendBufferRef sendBuffer = MakeShared<SendBuffer>(packetSize);
#else
		SendBufferRef sendBuffer = GSendBufferManager->Open(packetSize);
#endif

		PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
		header->size = packetSize;
		header->id = pktId;
		pkt.SerializeToArray(&header[1], dataSize);
		sendBuffer->Close(packetSize);

		return sendBuffer;
	}
};