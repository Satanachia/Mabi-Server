#pragma once

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.

#ifdef _DEBUG
#pragma comment(lib, "ServerCore\\Debug\\ServerCore.lib")
#pragma comment(lib, "ProtoBuf\\Debug\\libprotobufd.lib")
#else
#pragma comment(lib, "ServerCore\\Release\\ServerCore.lib")
#pragma comment(lib, "ProtoBuf\\Release\\libprotobuf.lib")
#endif // _DEBUG

#include "CorePch.h"
#include "Enum.pb.h"
#include "Protocol.pb.h"
#include "Struct.pb.h"
#include "ClientPacketHandler.h"
#include "Utility.h"
#include "UtilMath.h"


USING_SHARED_PTR(GameSession);
USING_SHARED_PTR(Room);
USING_SHARED_PTR(Player);
USING_SHARED_PTR(Monster);
USING_SHARED_PTR(Creature);
USING_SHARED_PTR(Object);
USING_SHARED_PTR(MonsterSpawner);

USING_SHARED_PTR(Projectile);
USING_SHARED_PTR(FieldItem);

USING_SHARED_PTR(BaseState);
USING_SHARED_PTR(StateMachine);

#define SEND_PACKERT(pkt)																							\
				SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);	\
				session->Send(sendBuffer);

template <typename ClassType>
std::weak_ptr<ClassType> GetWeak(std::shared_ptr<ClassType> ptr) { return std::weak_ptr<ClassType>(ptr); }

#define NULL_RETURN(__ptr) if(nullptr == __ptr)return
