#include "pch.h"
#include "DbManager.h"
#include "ThreadManager.h"
#include "DBSynchronizer.h"
#include "GenProcedures.h"
#include "GameSession.h"
#include "Player.h"

DbManagerRef GDbManager = nullptr;

DbManager::DbManager(int32 threadCount, const WCHAR* path)
	:isRun(false)
{
	std::cout << "DB Binding Start..." << std::endl;

	//DB Ŀ�ؼ� �����
	ASSERT_CRASH(pool.Connect(threadCount, path));

	DBConnection* dbConn = pool.Pop();
	DBSynchronizer dbSync(*dbConn);
	dbSync.Synchronize(L"GameDB.xml");
	pool.Push(dbConn);

	std::cout << "DB Binding Finish!" << std::endl;
	_pushOnlyValue = true;

	isRun.store(true);
	for (int32 i = 0; i < threadCount; ++i)
		GThreadManager->Launch(std::bind(&DbManager::AckQuery, this));
}

DbManager::~DbManager()
{
	isRun.store(false);
	_queries.Clear();
}

void DbManager::ReqQuery(std::function<void(DBConnection&)> _query)
{
	_queries.Push(_query);
}

void DbManager::AckQuery()
{
	DBConnection* connector = pool.Pop();

	while (true == isRun)
	{
		std::function<void(DBConnection&)> query = _queries.Pop();
		if (nullptr == query)
		{
			Sleep(0);
			continue;
		}
		
		query(*connector);
	}

	pool.Push(connector);
}

void DbManager::ExcuteSignUp(DBConnection& dbConn, std::weak_ptr<PacketSession> weakSessionRef, Protocol::C_LOGIN& pkt)
{
	std::wstring id = Utility::AnsiToUniCode(pkt.login_id());
	std::wstring pw = Utility::AnsiToUniCode(pkt.login_pw());
	
	//���̵� �ߺ� Ȯ��
	SP::GetAccountId getAccount(dbConn);
	getAccount.In_LoginId(id.data(), static_cast<int32>(id.size()));
	getAccount.In_Password(pw.data(), static_cast<int32>(pw.size()));
	
	int32 accountId = 0;
	getAccount.Out_AccountId(accountId);
	getAccount.Execute();
	getAccount.Fetch();

	//���̵� �ߺ��� ���
	if (0 != accountId)
	{
		ReserveJob([weakSessionRef]()
		{
			PacketSessionRef session = weakSessionRef.lock();
			if (nullptr == session)
				return;
			
			//���� ���θ� true�� �����ϰ� ������(players���� ������ ���� ����)
			Protocol::S_LOGIN loginPkt;
			loginPkt.set_success(false);
			SEND_PACKERT(loginPkt);
		});
		return;
	}
	

	SP::SignUp signUp(dbConn);
	signUp.In_LoginId(id.data(), static_cast<int32>(id.size()));
	signUp.In_Password(pw.data(), static_cast<int32>(pw.size()));
	signUp.Execute();

	
	//��Ŷ�� �����°� ��Ŀ�����忡�� ó��
	ReserveJob([weakSessionRef]()
	{
		PacketSessionRef session = weakSessionRef.lock();
		if (nullptr == session)
			return;

		//���� ���θ� true�� �����ϰ� ������(players���� ������ ���� ����)
		Protocol::S_LOGIN loginPkt;
		loginPkt.set_success(true);
		SEND_PACKERT(loginPkt);
	});

	GConsoleLogger->ReserveMsg(Color::GREEN, L"New User SignUp\n");
}

void DbManager::ExcuteLogIn(DBConnection& dbConn, std::weak_ptr<PacketSession> weakSessionRef, Protocol::C_LOGIN& pkt)
{
	SP::GetAccountId accountProc(dbConn);

	//�α��� ���̵�� ��й�ȣ�� DB�� ����ID �˾ƿ���
	std::wstring id = Utility::AnsiToUniCode(pkt.login_id());
	std::wstring pw = Utility::AnsiToUniCode(pkt.login_pw());
	accountProc.In_LoginId(id.data(), static_cast<int32>(id.size()));
	accountProc.In_Password(pw.data(), static_cast<int32>(pw.size()));

	//Fetch���Ŀ� ���⿡ ���� �����
	int32 accountId = 0;
	accountProc.Out_AccountId(accountId);
	accountProc.Execute();
	accountProc.Fetch();

	//���̵� ���� ���
	if (0 == accountId)
	{
		//�α��� ���� ��Ŷ ������
		ReserveJob([weakSessionRef]()
		{
			PacketSessionRef session = weakSessionRef.lock();
			if (nullptr == session)
				return;

			Protocol::S_LOGIN loginPkt;
			loginPkt.set_success(false);
			SEND_PACKERT(loginPkt);
		});
		
		GConsoleLogger->ReserveMsg(Color::RED, L"Unknown User Try LogIn\n");
		return;
	}



	//������ �����ߴ� ��� �ش� ������ db���̵� ����
	PacketSessionRef session = weakSessionRef.lock();
	if (nullptr == session)
		return;

	GameSessionRef gameSession = std::static_pointer_cast<GameSession>(session);
	gameSession->SetDbAccountId(accountId);

	//db�� accountId�� ĳ���� �ҷ�����
	SP::GetCharacters characterProc(dbConn);
	characterProc.In_AccountId(accountId);

	//������ �����ϴ� ĳ���͵��� ��������
	int32 jobType;
	wchar_t characterName[20] = { 0, };
	characterProc.Out_JobType(jobType);
	characterProc.Out_CharacterName(characterName);
	characterProc.Execute();

	 Protocol::S_LOGIN loginPkt;
	while (true == characterProc.Fetch())
	{
		Protocol::ObjectInfo* characterInfo = loginPkt.add_players();
		characterInfo->set_object_id(jobType);	//id ��� PlayerType�� �ִ´�

		std::string name = Utility::UniCodeToAnsi(characterName);
		characterInfo->set_object_name(name);
	}
	gameSession->SetDbPlayers(loginPkt);

	ReserveJob([weakSessionRef]()
	{
		PacketSessionRef session = weakSessionRef.lock();
		if (nullptr == session)
			return;
		
		Protocol::S_LOGIN loginPkt;
		GameSessionRef gameSession = std::static_pointer_cast<GameSession>(session);
		loginPkt = gameSession->GetDbPlayers();
		loginPkt.set_success(true);
		SEND_PACKERT(loginPkt);
	});
	
	GConsoleLogger->ReserveMsg(Color::GREEN, L"%d User LogIn. Character Count : %d\n", accountId, loginPkt.players_size());
}

void DbManager::FetchItems(DBConnection& dbConn, PlayerRef player)
{
	const std::string& multiName = player->GetObjectInfo().object_name();
	std::wstring wideName = Utility::AnsiToUniCode(multiName);
	int32 nameSize = static_cast<int32>(wideName.size());

	SP::GetItems getItems(dbConn);
	getItems.In_Name(wideName.data(), nameSize);
	int32 itemType = -1;
	int32 itemCount = -1;
	getItems.Out_ItemId(itemType);
	getItems.Out_Count(itemCount);
	getItems.Execute();
	
	while (true == getItems.Fetch())
	{
		ASSERT_CRASH(-1 != itemType);
		ASSERT_CRASH(-1 != itemCount);
		Protocol::ItemType type = static_cast<Protocol::ItemType>(itemType);
		ReserveJob(std::bind(&Player::AcquireItem, player, type, itemCount, true));
	}
	
	ReserveJob(std::bind(&Player::SendInitInventoryPacket, player));
}

void DbManager::FetchMoney(DBConnection& dbConn, PlayerRef player)
{
	//���߿�
	

}
