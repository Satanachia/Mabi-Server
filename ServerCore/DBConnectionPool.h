#pragma once
#include "DBConnection.h"

/*----------------------------------------
							DBConnectionPool
----------------------------------------*/

class DBConnectionPool
{
public:
	DBConnectionPool();
	~DBConnectionPool();

	DBConnectionPool(const DBConnectionPool& _Other) = delete;
	DBConnectionPool(DBConnectionPool&& _Other) noexcept = delete;
	DBConnectionPool& operator=(const DBConnectionPool& _Other) = delete;
	DBConnectionPool& operator=(const DBConnectionPool&& _Other) noexcept = delete;

	bool Connect(int32 connectionCount, const WCHAR* connectionString);
	void Clear();

	DBConnection* Pop();
	void Push(DBConnection* connection);

private:
	USE_LOCK;

	SQLHENV							_environment = SQL_NULL_HANDLE;
	Vector<DBConnection*>	_connections;
};

