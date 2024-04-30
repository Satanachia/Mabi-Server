#include "pch.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "ClientPacketHandler.h"
#include "Room.h"

void GameSession::OnConnected()
{
	GSessionManager.Add(std::static_pointer_cast<GameSession>(shared_from_this()));
}

void GameSession::OnRecvPacket(BYTE* buffer, int32 len)
{
	PacketSessionRef session = GetPacketSessionRef();
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
	//나중에 만약 분산서버로 만든다면 여기서 패킷아이디를 체크하고 어떻게 처리할지 또 결정
	
	ClientPacketHandler::HandlePacket(session, buffer, len);
}

void GameSession::OnSend(int32 len)
{
	//std::cout << "OnSend Len = " << len << std::endl;
}

void GameSession::OnDisconnected()
{
	GSessionManager.Remove(std::static_pointer_cast<GameSession>(shared_from_this()));
	GConsoleLogger->ReserveMsg(Color::GREEN, L"Client Disconnect\n");
}

void GameSession::LeaveGame()
{
	_player = nullptr;
	//Disconnect(L"LeaveGame");
}

void GameSession::SetDbAccountId(int32 dbAccountId)
{
	if (0 == dbAccountId)
		return;

	//_dbAccountId가 한번 설정되면 다른 값으로 변경되지 못하게 수정
	if (0 != _dbAccountId)
	{
		ASSERT_CRASH(_dbAccountId == dbAccountId);
		return;
	}

	//스레드에 안전하진 않아도, 로그인 할 때만 호출 될 것임
	_dbAccountId = dbAccountId;
}

