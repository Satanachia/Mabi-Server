#include "pch.h"
#include "DBConnection.h"

/*----------------------------------------
							DBConnection
----------------------------------------*/

DBConnection::DBConnection()
{

}

DBConnection::~DBConnection()
{

}

bool DBConnection::Connect(SQLHENV henv, const WCHAR* connectionString)
{
	//SQLHDBC핸들을 DB와 연동
	if (SQL_SUCCESS != ::SQLAllocHandle(SQL_HANDLE_DBC, henv, &_connection))
		return false;

	WCHAR stringBuffer[MAX_PATH] = { 0 };
	::wcscpy_s(stringBuffer, connectionString);
	
	WCHAR resultString[MAX_PATH] = { 0 };
	SQLSMALLINT resultStringLen = 0;

	//_connection을 DB에 연결
	SQLRETURN ret = ::SQLDriverConnectW(
		_connection,
		NULL,
		reinterpret_cast<SQLWCHAR*>(stringBuffer),
		_countof(stringBuffer),
		OUT reinterpret_cast<SQLWCHAR*>(resultString),
		_countof(resultString),
		OUT &resultStringLen,
		SQL_DRIVER_NOPROMPT);

	//_statement를 DB에 연동
	if (SQL_SUCCESS != ::SQLAllocHandle(SQL_HANDLE_STMT, _connection, &_statement))
		return false;

	return (SQL_SUCCESS == ret || SQL_SUCCESS_WITH_INFO == ret);
}

void DBConnection::Clear()
{
	if (_connection != SQL_NULL_HANDLE)
	{
		::SQLFreeHandle(SQL_HANDLE_DBC, _connection);
		_connection = SQL_NULL_HANDLE;
	}

	if (_statement != SQL_NULL_HANDLE)
	{
		::SQLFreeHandle(SQL_HANDLE_STMT, _statement);
		_statement = SQL_NULL_HANDLE;
	}
}

bool DBConnection::Execute(const WCHAR* query)
{
	SQLRETURN ret = ::SQLExecDirectW(_statement, (SQLWCHAR*)query, SQL_NTSL);
	if (SQL_SUCCESS == ret || SQL_SUCCESS_WITH_INFO == ret)
		return true;

	HandleError(ret);
	return false;
}

bool DBConnection::Fetch()
{
	SQLRETURN ret = SQLFetch(_statement);
	
	switch (ret)
	{
	case SQL_SUCCESS:
	case SQL_SUCCESS_WITH_INFO:
		return true;
	case SQL_NO_DATA:
		return false;
	case SQL_ERROR:
		HandleError(ret);
		return false;
	default:
		return false;
	}
}

int32 DBConnection::GetRowCount()
{
	SQLLEN count = 0;
	SQLRETURN ret = ::SQLRowCount(_statement, OUT & count);

	if (SQL_SUCCESS == ret || SQL_SUCCESS_WITH_INFO == ret)
		return static_cast<int32>(count);

	return -1;
}

void DBConnection::Unbind()
{
	::SQLFreeStmt(_statement, SQL_UNBIND);
	::SQLFreeStmt(_statement, SQL_RESET_PARAMS);
	::SQLFreeStmt(_statement, SQL_CLOSE);
}



bool DBConnection::BindParam(int32 paramIndex, bool* value, SQLLEN* index)
{
	return BindParam(paramIndex, SQL_C_TINYINT, SQL_TINYINT, size32(bool), value, index);
}

bool DBConnection::BindParam(int32 paramIndex, float* value, SQLLEN* index)
{
	return BindParam(paramIndex, SQL_C_FLOAT, SQL_REAL, 0, value, index);
}

bool DBConnection::BindParam(int32 paramIndex, double* value, SQLLEN* index)
{
	return BindParam(paramIndex, SQL_C_DOUBLE, SQL_DOUBLE, 0, value, index);
}

bool DBConnection::BindParam(int32 paramIndex, int8* value, SQLLEN* index)
{
	return BindParam(paramIndex, SQL_C_TINYINT, SQL_TINYINT, size32(int8), value, index);
}

bool DBConnection::BindParam(int32 paramIndex, int16* value, SQLLEN* index)
{
	return BindParam(paramIndex, SQL_C_SHORT, SQL_SMALLINT, size32(int16), value, index);
}

bool DBConnection::BindParam(int32 paramIndex, int32* value, SQLLEN* index)
{
	return BindParam(paramIndex, SQL_C_LONG, SQL_INTEGER, size32(int32), value, index);
}

bool DBConnection::BindParam(int32 paramIndex, int64* value, SQLLEN* index)
{
	return BindParam(paramIndex, SQL_C_SBIGINT, SQL_BIGINT, size32(int64), value, index);
}

bool DBConnection::BindParam(int32 paramIndex, TIMESTAMP_STRUCT* value, SQLLEN* index)
{
	return BindParam(paramIndex, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, size32(TIMESTAMP_STRUCT), value, index);
}

bool DBConnection::BindParam(int32 paramIndex, const WCHAR* str, SQLLEN* index)
{
	SQLULEN size = static_cast<SQLULEN>((::wcslen(str) + 1) * 2);
	*index = SQL_NTSL;

	if (size > WVARCHAR_MAX)
		return BindParam(paramIndex, SQL_C_WCHAR, SQL_WLONGVARCHAR, size, (SQLPOINTER)str, index);
	else
		return BindParam(paramIndex, SQL_C_WCHAR, SQL_WVARCHAR, size, (SQLPOINTER)str, index);
}

bool DBConnection::BindParam(int32 paramIndex, const BYTE* bin, int32 size, SQLLEN* index)
{
	if (bin == nullptr)
	{
		*index = SQL_NULL_DATA;
		size = 1;
	}
	else
		*index = size;	//가변 데이터 크기 수정

	if (size > BINARY_MAX)
		return BindParam(paramIndex, SQL_C_BINARY, SQL_LONGVARBINARY, size, (BYTE*)bin, index);
	else
		return BindParam(paramIndex, SQL_C_BINARY, SQL_BINARY, size, (BYTE*)bin, index);
}

bool DBConnection::BindCol(int32 columnIndex, bool* value, SQLLEN* index)
{
	return BindCol(columnIndex, SQL_C_TINYINT, size32(bool), value, index);
}

bool DBConnection::BindCol(int32 columnIndex, float* value, SQLLEN* index)
{
	return BindCol(columnIndex, SQL_C_FLOAT, size32(float), value, index);
}

bool DBConnection::BindCol(int32 columnIndex, double* value, SQLLEN* index)
{
	return BindCol(columnIndex, SQL_C_DOUBLE, size32(double), value, index);
}

bool DBConnection::BindCol(int32 columnIndex, int8* value, SQLLEN* index)
{
	return BindCol(columnIndex, SQL_C_TINYINT, size32(int8), value, index);
}

bool DBConnection::BindCol(int32 columnIndex, int16* value, SQLLEN* index)
{
	return BindCol(columnIndex, SQL_C_SHORT, size32(int16), value, index);
}

bool DBConnection::BindCol(int32 columnIndex, int32* value, SQLLEN* index)
{
	return BindCol(columnIndex, SQL_C_LONG, size32(int32), value, index);
}

bool DBConnection::BindCol(int32 columnIndex, int64* value, SQLLEN* index)
{
	return BindCol(columnIndex, SQL_C_SBIGINT, size32(int64), value, index);
}

bool DBConnection::BindCol(int32 columnIndex, TIMESTAMP_STRUCT* value, SQLLEN* index)
{
	return BindCol(columnIndex, SQL_C_TYPE_TIMESTAMP, size32(TIMESTAMP_STRUCT), value, index);
}

bool DBConnection::BindCol(int32 columnIndex, WCHAR* str, int32 size, SQLLEN* index)
{
	return BindCol(columnIndex, SQL_C_WCHAR, size, str, index);
}

bool DBConnection::BindCol(int32 columnIndex, BYTE* bin, int32 size, SQLLEN* index)
{
	return BindCol(columnIndex, SQL_BINARY, size, bin, index);
}



bool DBConnection::BindParam(
	SQLUSMALLINT paramIndex,		
	SQLSMALLINT cType,					
	SQLSMALLINT sqlType, 				
	SQLULEN len,								
	SQLPOINTER ptr,						
	SQLLEN* index)							
{
	SQLRETURN ret= ::SQLBindParameter(
		_statement,
		paramIndex,
		SQL_PARAM_INPUT,
		cType, sqlType,len, 
		0, ptr, 0, index);

	if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
	{
		HandleError(ret);
		return false;
	}

	return true;
}

bool DBConnection::BindCol(
	SQLUSMALLINT columnIndex, 
	SQLSMALLINT cType, 
	SQLLEN len, SQLPOINTER value, 
	SQLLEN* index)
{
	SQLRETURN ret = ::SQLBindCol(_statement, columnIndex, cType, value, len, index);
	if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
	{
		HandleError(ret);
		return false;
	}

	return true;
}

void DBConnection::HandleError(SQLRETURN ret)
{
	//예외처리
	if (SQL_SUCCESS == ret)
		return;

	SQLSMALLINT index = 1;
	SQLWCHAR sqlState[MAX_PATH] = { 0 };
	SQLINTEGER nativeErr = 0;
	SQLWCHAR errMsg[MAX_PATH] = { 0 };
	SQLSMALLINT msgLen = 0;
	SQLRETURN errorRet = 0;

	while (true)
	{
		//에러 사유 확인
		errorRet= ::SQLGetDiagRecW(
			SQL_HANDLE_STMT,
			_statement,
			index,
			sqlState,
			OUT & nativeErr,
			errMsg,
			_countof(errMsg),
			OUT & msgLen);

		//아무런 문제 없는 경우
		if (SQL_NO_DATA == errorRet)
			break;

		if (SQL_SUCCESS != errorRet && SQL_SUCCESS_WITH_INFO != errorRet)
			break;

		std::wcout.imbue(std::locale("kor"));
		std::wcout << errMsg << std::endl;

		++index;
	}
}
