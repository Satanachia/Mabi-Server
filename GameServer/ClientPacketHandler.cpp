#include "pch.h"
#include "ClientPacketHandler.h"
#include "GameSession.h"
#include "Player.h"
#include "Room.h"
#include "ObjectUtils.h"
#include "DbManager.h"
#include "GenProcedures.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

#define CHECK_PLAYER_AND_ROOM(session)								\
								PlayerRef player = session->GetPlayer();		\
								if (nullptr == player)										\
									return false;												\
								RoomRef room = player->GetRoom();			\
								if (nullptr == room)											\
									return false;

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
	//TODO : Log
	return false;
}


/*----------------------------------------------------------------------
										로비
----------------------------------------------------------------------*/

bool Handle_C_LOGIN(PacketSessionRef& session, Protocol::C_LOGIN& pkt)
{
	//로그인이 아니라 회원가입인 경우
	if (true == pkt.sign_up())
		GDbManager->ReqQuery(std::bind(&DbManager::ExcuteSignUp, GDbManager, std::placeholders::_1, session, pkt));
	
	//로그인인 경우
	else
		GDbManager->ReqQuery(std::bind(&DbManager::ExcuteLogIn, GDbManager, std::placeholders::_1, session, pkt));
	
	return true;
}

bool Handle_C_CREATE_NEW_PLAYER(PacketSessionRef& session, Protocol::C_CREATE_NEW_PLAYER& pkt)
{
	GameSessionRef gameSession = std::static_pointer_cast<GameSession>(session);
	Protocol::CreatureType jobType = pkt.player_type();
	const std::string& name = pkt.player_name();
	
	//이름 중복 확인?
	

	//방에 입장시키기
	PlayerRef player = ObjectUtils::CreatePlayer(gameSession, jobType, name);
	ObjectRef object = player;
	RoomRef startRoom = GRoomManager.GetRoom(Protocol::LEVEL_TYPE_START_BRIDGE);
	startRoom->DoASync(&Room::EnterRoom, object, true, Protocol::PosInfo());
	
	
	//db에 데이터 삽입
	const std::wstring& unicodeName = Utility::AnsiToUniCode(name);
	GDbManager->ReqQuery([jobType, unicodeName, gameSession](DBConnection& dbConn)
	{
		SP::CreateNewCharacter insertNewPlayerProc(dbConn);
		insertNewPlayerProc.In_AccountId(gameSession->GetDbAccountId());
		insertNewPlayerProc.In_Type(static_cast<int32>(jobType));
		insertNewPlayerProc.In_Name(unicodeName.data(), static_cast<int32>(unicodeName.size()));

		ASSERT_CRASH(insertNewPlayerProc.Execute());
		GConsoleLogger->ReserveMsg(Color::BLUE, L"Insert New Player\n");
	});
	
	//어차피 안 쓸 것 같아서 추가 안함
	//gameSession->SetDbPlayers()
	return false;
}

bool Handle_C_ENTER_GAME(PacketSessionRef& session, Protocol::C_ENTER_GAME& pkt)
{
	GameSessionRef gameSession = std::static_pointer_cast<GameSession>(session);
	const Protocol::ObjectInfo& players = gameSession->GetDbPlayers().players(pkt.playerindex());
	
	//로비화면까지는 object_id에 플레이어 직업타입이 들어가있음
	PlayerRef player = ObjectUtils::CreatePlayer(gameSession, static_cast<Protocol::CreatureType>(players.object_id()), players.object_name());
	player->FetchInventory();

	//플레이어 만들고 Room의 JobQueue에 입장 함수객체 넣어주기
	ObjectRef object = player;
	RoomRef room = GRoomManager.GetRoom(Protocol::LEVEL_TYPE_START_BRIDGE);
	room->DoASync(&Room::EnterRoom, object, true, Protocol::PosInfo());
	GConsoleLogger->ReserveMsg(Color::GREEN, L"New Player Enter Game\n");
	return true;
}

bool Handle_C_LEAVE_GAME(PacketSessionRef& session, Protocol::C_LEAVE_GAME& pkt)
{
	GameSessionRef gameSession = std::static_pointer_cast<GameSession>(session);
	CHECK_PLAYER_AND_ROOM(gameSession);
	
	room->DoASync([player]()
	{
		player->SaveItems();
	});
	
	ObjectRef object = player;
	room->DoASync(&Room::LeaveRoom, object);
	room->DoASync([gameSession]()
	{
		gameSession->LeaveGame();
	});

	return true;
}


/*----------------------------------------------------------------------
										필드
----------------------------------------------------------------------*/

bool Handle_C_CHANGE_LEVEL(PacketSessionRef& session, Protocol::C_CHANGE_LEVEL& pkt)
{
	GameSessionRef gameSession = std::static_pointer_cast<GameSession>(session);
	PlayerRef player = gameSession->GetPlayer();
	if (nullptr == player)
		return false;
	
	RoomRef prevRoom = player->GetRoom();
	ObjectRef object = player;
	if (nullptr != prevRoom)
	{
		prevRoom->DoASync(&Room::LeaveRoom, object);
	}
	
	RoomRef nextRoom = GRoomManager.GetRoom(pkt.next_level());
	nextRoom->DoASync(&Room::EnterRoom, object, false, pkt.spawn_pos());
	return true;
}

bool Handle_C_MOVE(PacketSessionRef& session, Protocol::C_MOVE& pkt)
{
	GameSessionRef gameSession = std::static_pointer_cast<GameSession>(session);
	CHECK_PLAYER_AND_ROOM(gameSession);

	room->DoASync([player, pkt]()
	{
		player->HandleMove(pkt);
	});
	return true;
}

bool Handle_C_LOOK(PacketSessionRef& session, Protocol::C_LOOK& pkt)
{
	GameSessionRef gameSession = std::static_pointer_cast<GameSession>(session);
	CHECK_PLAYER_AND_ROOM(gameSession);
	
	room->DoASync([player, pkt]()
	{
		RoomRef room = player->GetRoom();
		NULL_RETURN(room);

		Protocol::S_LOOK lookPkt;
		lookPkt.set_object_id(player->GetObjectInfo().object_id());
		if (true == pkt.has_look_pos())
		{
			lookPkt.mutable_look_pos()->CopyFrom(pkt.look_pos());
		}
		else
		{
			lookPkt.set_yaw(pkt.yaw());
		}
		
		SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(lookPkt);
		room->Broadcast(sendBuffer);
	});

	return true;
}

bool Handle_C_ANIMATION(PacketSessionRef& session, Protocol::C_ANIMATION& pkt)
{
	GameSessionRef gameSession = std::static_pointer_cast<GameSession>(session);
	CHECK_PLAYER_AND_ROOM(gameSession);

	room->DoASync([player, pkt]()
	{
		RoomRef room = player->GetRoom();
		NULL_RETURN(room);

		Protocol::S_ANIMATION aniPkt;
		aniPkt.set_ani_type(pkt.ani_type());
		aniPkt.set_object_id(player->GetObjectInfo().object_id());
		SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(aniPkt);
		room->Broadcast(sendBuffer);
	});

	return true;
}

/*----------------------------------------------------------------------
										전투
----------------------------------------------------------------------*/

bool Handle_C_ATTACK(PacketSessionRef& session, Protocol::C_ATTACK& pkt)
{
	GameSessionRef gameSession = std::static_pointer_cast<GameSession>(session);
	CHECK_PLAYER_AND_ROOM(gameSession);

	std::weak_ptr<Player> playerWeak = GetWeak<Player>(player);
	room->DoASync([playerWeak, pkt]()
	{
		PlayerRef player = playerWeak.lock();
		if (nullptr == player)
			return;
		
		player->HandleAttack(pkt);
	});

	return true;
}

bool Handle_C_THINK(PacketSessionRef& session, Protocol::C_THINK& pkt)
{
	enum
	{
		MAGE_READY_TIME = 3000,
		WARRIOR_READY_TIME = 1500,
		ROG_READY_TIME = 500,
	};

	Protocol::AttackType attackType = pkt.attack_type();
	uint64 waitTime = 0;
	switch (attackType)
	{
	case Protocol::ATTACK_TYPE_SKILL_MAGE:
		waitTime = MAGE_READY_TIME;
		break;
	case Protocol::ATTACK_TYPE_SKILL_WARRIOR:
		waitTime = WARRIOR_READY_TIME;
		break;
	case Protocol::ATTACK_TYPE_SKILL_ROG:
		waitTime = ROG_READY_TIME;
		break;
	}

	if (0 == waitTime)
		return false;

	GameSessionRef gameSession = std::static_pointer_cast<GameSession>(session);
	CHECK_PLAYER_AND_ROOM(gameSession);

	uint64 objectId = player->GetObjectInfo().object_id();
	uint64 dummyID = 0;
	
	Protocol::S_THINK actionReqPkt;
	actionReqPkt.set_attack_type(attackType);
	actionReqPkt.set_object_id(objectId);
	SendBufferRef reqSendBuffer = ClientPacketHandler::MakeSendBuffer(actionReqPkt);
	room->DoASync(&Room::Broadcast, reqSendBuffer, objectId);

	Protocol::S_THINK_DONE actionAckPkt;
	actionAckPkt.set_attack_type(attackType);
	actionAckPkt.set_object_id(objectId);
	SendBufferRef ackSendBuffer = ClientPacketHandler::MakeSendBuffer(actionAckPkt);
	room->DoTimer(waitTime, &Room::Broadcast, ackSendBuffer, dummyID);
	return true;
}

bool Handle_C_SPAWN_PARTICLE(PacketSessionRef& session, Protocol::C_SPAWN_PARTICLE& pkt)
{
	GameSessionRef gameSession = std::static_pointer_cast<GameSession>(session);
	CHECK_PLAYER_AND_ROOM(gameSession);

	Protocol::S_SPAWN_PARTICLE particlePkt;
	particlePkt.set_level(room->GetRoomType());
	particlePkt.mutable_position()->CopyFrom(pkt.position());
	particlePkt.set_yaw(pkt.yaw());
	particlePkt.set_particle_type(pkt.particle_type());
	SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(particlePkt);
	uint64 dummyID = 0;
	room->DoASync(&Room::Broadcast, sendBuffer, dummyID);

	return true;
}


/*----------------------------------------------------------------------
										인벤토리
----------------------------------------------------------------------*/

bool Handle_C_DROP_ITEM(PacketSessionRef& session, Protocol::C_DROP_ITEM& pkt)
{
	GameSessionRef gameSession = std::static_pointer_cast<GameSession>(session);
	CHECK_PLAYER_AND_ROOM(gameSession);
	
	std::weak_ptr<Player> playerWeak = GetWeak<Player>(player);
	room->DoASync([playerWeak, pkt]()
	{
		PlayerRef player = playerWeak.lock();
		if (nullptr == player)
			return;
		
		player->HandleDropItem(pkt);
	});

	return true;
}

bool Handle_C_ADD_ITEM(PacketSessionRef& session, Protocol::C_ADD_ITEM& pkt)
{
	GameSessionRef gameSession = std::static_pointer_cast<GameSession>(session);
	CHECK_PLAYER_AND_ROOM(gameSession);

	std::weak_ptr<Player> playerWeak = GetWeak<Player>(player);
	room->DoASync([playerWeak, pkt]()
	{
		PlayerRef player = playerWeak.lock();
		if (nullptr == player)
			return;
		
		player->HandleAddItem(pkt);
	});

	return true;
}


bool Handle_C_USE_ITEM(PacketSessionRef& session, Protocol::C_USE_ITEM& pkt)
{
	GameSessionRef gameSession = std::static_pointer_cast<GameSession>(session);
	CHECK_PLAYER_AND_ROOM(gameSession);

	std::weak_ptr<Player> playerWeak = GetWeak<Player>(player);
	room->DoASync([playerWeak, pkt]()
	{
		PlayerRef player = playerWeak.lock();
		if (nullptr == player)
			return;

		player->HandleUseItem(pkt);
	});

	return true;
}



/*----------------------------------------------------------------------
										채팅
----------------------------------------------------------------------*/

bool Handle_C_PING(PacketSessionRef& session, Protocol::C_PING& pkt)
{
	GameSessionRef gameSession = std::static_pointer_cast<GameSession>(session);
	CHECK_PLAYER_AND_ROOM(gameSession);

	room->DoASync([session]()
	{
		Protocol::S_PONG reply;
		SEND_PACKERT(reply);
	});

	return true;
}

bool Handle_C_CHAT(PacketSessionRef& session, Protocol::C_CHAT& pkt) 
{
	std::cout << pkt.msg() << std::endl;

	/*Protocol::S_CHAT charPkt;
	charPkt.set_msg(pkt.msg());
	SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(charPkt);
	GRoom->DoASync(&Room::Broadcast, sendBuffer);*/

	return true;
}

