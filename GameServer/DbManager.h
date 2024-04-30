#pragma once
#include "DBConnectionPool.h"
#include "Protocol.pb.h"
#include "JobQueue.h"

class DbManager : public JobQueue
{
public:
	DbManager(int32 threadCount, const WCHAR* path);
	~DbManager();

	DbManager(const DbManager& _Other) = delete;
	DbManager(DbManager&& _Other) noexcept = delete;
	DbManager& operator=(const DbManager& _Other) = delete;
	DbManager& operator=(const DbManager&& _Other) noexcept = delete;
	
public:
	void ReqQuery(std::function<void(DBConnection&)> _query);

	//db스레드에서 처리하고 싶지 않는 작업들을 글로벌 큐에 맡기는 함수
	void ReserveJob(CallbackType&& callback)
	{
		//db스레드에서 처리하기 위해 pushOnly = true
		Push(ObjectPool<Job>::MakeShared(std::move(callback)), true);
	}

private:
	void AckQuery();

public:
	/*--------------------------------------
						컨텐츠
	--------------------------------------*/

	void ExcuteSignUp(DBConnection& dbConn, std::weak_ptr<PacketSession> weakSessionRef, Protocol::C_LOGIN& pkt);
	void ExcuteLogIn(DBConnection& dbConn, std::weak_ptr<PacketSession> weakSessionRef, Protocol::C_LOGIN& pkt);
	void FetchItems(DBConnection& dbConn, PlayerRef player);
	void FetchMoney(DBConnection& dbConn, PlayerRef player);

	template<typename DBProc>
	void SaveItem(DBConnection& dbConn, std::wstring playerName, Protocol::ItemType itemType, int32 itemCount)
	{
		const wchar_t* namePtr = playerName.data();
		int32 nameSize = static_cast<int32>(playerName.size());

		DBProc proc(dbConn);
		proc.In_Name(namePtr, nameSize);
		proc.In_ItemId(itemType);
		proc.In_Count(itemCount);
		ASSERT_CRASH(proc.Execute());
	}

private:
	LockQueue<std::function<void(DBConnection& connector)>> _queries;
	DBConnectionPool pool;
	Atomic<bool> isRun;
};

USING_SHARED_PTR(DbManager);
extern DbManagerRef GDbManager;
