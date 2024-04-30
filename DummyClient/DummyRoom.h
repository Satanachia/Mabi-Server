#pragma once
#include "JobQueue.h"
#include "Protocol.pb.h"
#include <random>


class DummyPlayer
{
public:
	uint64 id;
	Protocol::PosInfo nowPos;
	std::weak_ptr<class PacketSession> session;
};

USING_SHARED_PTR(DummyPlayer);

class DummyRoom : public JobQueue
{
public:
	DummyRoom();
	~DummyRoom();

	DummyRoom(const DummyRoom& _Other) = delete;
	DummyRoom(DummyRoom&& _Other) noexcept = delete;
	DummyRoom& operator=(const DummyRoom& _Other) = delete;
	DummyRoom& operator=(const DummyRoom&& _Other) noexcept = delete;

	void Enter(DummyPlayerRef player);
	void Move(uint64 id);
	
	template <typename T>
	static T GetRandom(T min, T max)
	{
		std::random_device randomDevice;
		std::mt19937 generator(randomDevice());

		//constexpr는 컴파일 타임에 분기처리를 하여 템플릿 함수를 만들어 낸다

		//T가 int인 경우
		if constexpr (std::is_integral_v<T>)
		{
			std::uniform_int_distribution<T> distribution(min, max);
			return distribution(generator);
		}

		//T가 float인 경우
		else
		{
			std::uniform_real_distribution<T> distribution(min, max);
			return distribution(generator);
		}
	}

private:
	HashMap<uint64, DummyPlayerRef> _objects;
};

USING_SHARED_PTR(DummyRoom);
extern DummyRoomRef GRoom;