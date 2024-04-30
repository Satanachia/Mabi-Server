#pragma once
#include "Session.h"
#include "Protocol.pb.h"

class GameSession : public PacketSession
{
public:
	~GameSession()
	{
	}

	virtual void OnConnected() override;
	virtual void OnRecvPacket(BYTE* buffer, int32 len) override;
	virtual void OnSend(int32 len) override;
	virtual void OnDisconnected() override;

	void LeaveGame();
	void SetPlayer(PlayerRef player) { _player = player; }
	PlayerRef GetPlayer() { return _player.load(); }

	void SetDbAccountId(int32 dbAccountId);
	int32 GetDbAccountId() { return _dbAccountId; }

	void SetDbPlayers(const Protocol::S_LOGIN& players) { _dbPlayers = players; }
	const Protocol::S_LOGIN& GetDbPlayers() { return _dbPlayers; }

private:
	Atomic<PlayerRef> _player;
	int32 _dbAccountId = 0;
	Protocol::S_LOGIN _dbPlayers;
};

