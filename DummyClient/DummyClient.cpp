#include "pch.h"
#include <iostream>

#include "ThreadManager.h"
#include "Service.h"
#include "Session.h"
#include "BufferReader.h"
#include "ServerPacketHandler.h"

char sendData[] = "Hello World";

class ServerSession : public PacketSession
{
public:
	~ServerSession()
	{
		std::cout << "~ServerSession" << std::endl;
	}

	virtual void OnConnected() override
	{
		Protocol::C_LOGIN pkt;
		pkt.set_login_id("test");
		pkt.set_login_pw("123123");
		pkt.set_sign_up(false);
		SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
		Send(sendBuffer);
	}

	virtual void OnRecvPacket(BYTE* buffer, int32 len) override
	{
		PacketSessionRef session = GetPacketSessionRef();
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		//나중에 만약 분산서버로 만든다면 여기서 패킷아이디를 체크하고 어떻게 처리할지 또 결정

		ServerPacketHandler::HandlePacket(session, buffer, len);
	}

	virtual void OnSend(int32 len) override
	{
		//std::cout << "OnSend Len = " << len << std::endl;
	}

	virtual void OnDisconnected() override
	{
		std::cout << "Disconnected" << std::endl;
	}
};

int main()
{
	int connectCount;
	std::cout << "Connect Count : ";
	std::cin >> connectCount;
	
	ServerPacketHandler::Init();
	//std::this_thread::sleep_for(std::chrono::seconds(3));
	
	ClientServiceRef service = MakeShared<ClientService>(
		NetAddress(L"127.0.0.1", 7777),
		MakeShared<IocpCore>(),
		MakeShared<ServerSession>, connectCount);

	ASSERT_CRASH(service->Start());
	
	
	for (int32 i = 0; i < 2; i++)
	{
		GThreadManager->Launch([service]()
		{
			while (true)
			{
				LEndTickCount = ::GetTickCount64() + 64;

				//네트워크 입출력 처리 -> 인게임 로직
				service->GetIocpCore()->Dispatch(10);

				//JobTimer의 예약된 일감이 시간이 지났다면 JobQueue에 일감 넣기
				ThreadManager::DistributeReserveJobs();

				//글로벌 큐의 Job Queue처리
				ThreadManager::DoGlobalQueueWork();
			}
		});
	}

	//Protocol::C_CHAT chatPkt;
	//chatPkt.set_msg(u8"Hello World!");
	//SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(chatPkt);
	while (true)
	{
		//service->Broadcast(sendBuffer);
		//std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	GThreadManager->Join();
	return 0;
}