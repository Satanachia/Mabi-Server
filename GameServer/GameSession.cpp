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
	//���߿� ���� �л꼭���� ����ٸ� ���⼭ ��Ŷ���̵� üũ�ϰ� ��� ó������ �� ����
	
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

	//_dbAccountId�� �ѹ� �����Ǹ� �ٸ� ������ ������� ���ϰ� ����
	if (0 != _dbAccountId)
	{
		ASSERT_CRASH(_dbAccountId == dbAccountId);
		return;
	}

	//�����忡 �������� �ʾƵ�, �α��� �� ���� ȣ�� �� ����
	_dbAccountId = dbAccountId;
}

