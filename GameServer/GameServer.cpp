#include "pch.h"
#include <iostream>

#include "ThreadManager.h"
#include "Service.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "BufferWriter.h"
#include "ClientPacketHandler.h"
#include "Protocol.pb.h"
#include "Job.h"
#include "Room.h"
#include "GenProcedures.h"

#include "DBBind.h"
#include "XmlParser.h"
#include "DbManager.h"

enum
{
	WORKER_TICK = 64
};

void DoWorkerJob(ServerServiceRef& service)
{
	while (true)
	{
		LEndTickCount = ::GetTickCount64() + WORKER_TICK;
		
		//네트워크 입출력 처리 -> 인게임 로직
		service->GetIocpCore()->Dispatch(10);

		//JobTimer의 예약된 일감이 시간이 지났다면 JobQueue에 일감 넣기
		ThreadManager::DistributeReserveJobs();

		//글로벌 큐의 Job Queue처리
		ThreadManager::DoGlobalQueueWork();
	}
}

int main()
{
	//DB연동(DB 매니저는 오브젝트 풀 안씀)
	GDbManager = std::make_shared<DbManager>(2, L"Driver={SQL Server Native Client 11.0};Server=(localdb)\\ProjectModels;Database=ServerDb;Trusted_Connection=Yes;");


#pragma region DB 연습

	/*회원가입하기
	GDbManager.ReqQuery([](DBConnection* dbConn)
	{
		SP::SignUp signUp(*dbConn);
		signUp.In_LoginId(L"admin");
		signUp.In_Password(L"123123");
		signUp.Execute();
	});*/



	//캐릭터 만들기
	//GDbManager.ReqQuery([](DBConnection& dbConn)
	//{
	//	/*SP::GetAccountId accountProc(dbConn);
	//	std::wstring id = L"admin";
	//	std::wstring pw = L"123123";
	//	accountProc.In_LoginId(L"admin");
	//	accountProc.In_Password(L"123123");*/

	//	int32 accountId = 2;
	//	/*accountProc.Out_AccountId(accountId);
	//	accountProc.Execute();
	//	accountProc.Fetch();*/

	//	SP::CreateNewCharacter newCharacter(dbConn);
	//	newCharacter.In_AccountId(accountId);
	//	newCharacter.In_Type(1);
	//	newCharacter.In_Name(L"MyWarrior");
	//	bool ret = newCharacter.Execute();
	//});


	/*GDbManager.ReqQuery([](DBConnection* dbConn)
	{
		WCHAR name[] = L"insetour";

		SP::InsertGold insertGold(*dbConn);
		insertGold.In_Gold(100);
		insertGold.In_Name(name);
		insertGold.In_CreateData(TIMESTAMP_STRUCT{ 2024, 2, 6 });
		insertGold.Execute();
	});

	GDbManager.ReqQuery([](DBConnection* dbConn)
	{
		SP::GetGold getGold(*dbConn);
		getGold.In_Gold(100);

		int32 id = 0;
		int32 gold = 0;
		WCHAR name[100];
		TIMESTAMP_STRUCT date;

		getGold.Out_Id(OUT id);
		getGold.Out_Gold(OUT gold);
		getGold.Out_Name(OUT name);
		getGold.Out_CreateDate(OUT date);

		getGold.Execute();

		while (getGold.Fetch())
		{
			GConsoleLogger->WriteStdOut(Color::BLUE,
				L"ID[%d] Gold[%d] Name[%s]\n", id, gold, name);
		}
	});*/

	/*ASSERT_CRASH(GDBConnectionPool->Connect(1, L"Driver={SQL Server Native Client 11.0};Server=(localdb)\\ProjectModels;Database=ServerDb;Trusted_Connection=Yes;"));

	DBConnection* dbConn = GDBConnectionPool->Pop();
	DBSynchronizer dbSync(*dbConn);
	dbSync.Synchronize(L"GameDB.xml");

	{
		WCHAR name[] = L"insetour";

		SP::InsertGold insertGold(*dbConn);
		insertGold.In_Gold(100);
		insertGold.In_Name(name);
		insertGold.In_CreateData(TIMESTAMP_STRUCT{ 2024, 2, 6 });
		insertGold.Execute();
	}

	{
		SP::GetGold getGold(*dbConn);
		getGold.In_Gold(100);

		int32 id = 0;
		int32 gold = 0;
		WCHAR name[100];
		TIMESTAMP_STRUCT date;

		getGold.Out_Id(OUT id);
		getGold.Out_Gold(OUT gold);
		getGold.Out_Name(OUT name);
		getGold.Out_CreateDate(OUT date);

		getGold.Execute();

		while (getGold.Fetch())
		{
			GConsoleLogger->WriteStdOut(Color::BLUE,
				L"ID[%d] Gold[%d] Name[%s]\n", id, gold, name);
		}
	}*/

#pragma endregion


	ClientPacketHandler::Init();

	ServerServiceRef service = MakeShared<ServerService>(
		NetAddress(L"127.0.0.1", 7777),
		MakeShared<IocpCore>(),
		MakeShared<GameSession>, 100);

	ASSERT_CRASH(service->Start());


	for (int32 i = 0; i < 5; i++)
	{
		GThreadManager->Launch([&service]()
		{
			DoWorkerJob(service);
		});
	}

	//MainThread
	DoWorkerJob(service);
	GThreadManager->Join();
	return 0;
}

