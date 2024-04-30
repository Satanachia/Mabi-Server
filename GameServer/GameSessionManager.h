#pragma once

class GameSession;

class GameSessionManager
{
public:
	GameSessionManager();
	~GameSessionManager();

	GameSessionManager(const GameSessionManager& _Other) = delete;
	GameSessionManager(GameSessionManager&& _Other) noexcept = delete;
	GameSessionManager& operator=(const GameSessionManager& _Other) = delete;
	GameSessionManager& operator=(const GameSessionManager&& _Other) noexcept = delete;

	void Add(GameSessionRef session);
	void Remove(GameSessionRef session);
	void Broadcast(SendBufferRef sendBuffer);

private:
	USE_LOCK;
	Set<GameSessionRef> _sessions;
};

extern GameSessionManager GSessionManager;
