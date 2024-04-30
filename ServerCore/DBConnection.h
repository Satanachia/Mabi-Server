#pragma once
#include <sql.h>
#include <sqlext.h>

/*----------------------------------------
							DBConnection
----------------------------------------*/

enum
{
	WVARCHAR_MAX = 4000,
	BINARY_MAX = 8000,
};

class DBConnection
{
public:
	DBConnection();
	~DBConnection();

	DBConnection(const DBConnection& _Other) = delete;
	DBConnection(DBConnection&& _Other) noexcept = delete;
	DBConnection& operator=(const DBConnection& _Other) = delete;
	DBConnection& operator=(const DBConnection&& _Other) noexcept = delete;

	bool Connect(SQLHENV henv, const WCHAR* connectionString);
	void Clear();

	//실제 쿼리를 실행하는 함수
	bool Execute(const WCHAR* query);

	//SELECT 문 처럼 결과를 받아올 때 사용하는 함수
	bool Fetch();

	//행의 갯수 가져오기
	int32 GetRowCount();

	//BindParam/BindCol을 실행했을때 바인드된 내용을 정리(초기화)
	void Unbind();

public:
	bool BindParam(int32 paramIndex, bool* value, SQLLEN* index);
	bool BindParam(int32 paramIndex, float* value, SQLLEN* index);
	bool BindParam(int32 paramIndex, double* value, SQLLEN* index);
	bool BindParam(int32 paramIndex, int8* value, SQLLEN* index);
	bool BindParam(int32 paramIndex, int16* value, SQLLEN* index);
	bool BindParam(int32 paramIndex, int32* value, SQLLEN* index);
	bool BindParam(int32 paramIndex, int64* value, SQLLEN* index);
	bool BindParam(int32 paramIndex, TIMESTAMP_STRUCT* value, SQLLEN* index);
	bool BindParam(int32 paramIndex, const WCHAR* str, SQLLEN* index);
	bool BindParam(int32 paramIndex, const BYTE* bin, int32 size, SQLLEN* index);

	bool BindCol(int32 columnIndex, bool* value, SQLLEN* index);
	bool BindCol(int32 columnIndex, float* value, SQLLEN* index);
	bool BindCol(int32 columnIndex, double* value, SQLLEN* index);
	bool BindCol(int32 columnIndex, int8* value, SQLLEN* index);
	bool BindCol(int32 columnIndex, int16* value, SQLLEN* index);
	bool BindCol(int32 columnIndex, int32* value, SQLLEN* index);
	bool BindCol(int32 columnIndex, int64* value, SQLLEN* index);
	bool BindCol(int32 columnIndex, TIMESTAMP_STRUCT* value, SQLLEN* index);
	bool BindCol(int32 columnIndex, WCHAR* str, int32 size, SQLLEN* index);
	bool BindCol(int32 columnIndex, BYTE* bind, int32 size, SQLLEN* index);

private:
	/// <summary>
	/// 쿼리를 실행시키기 전에 인자를 바인딩해주는 함수
	/// </summary>
	/// <param name="paramIndex">몇번째 인자로 데이터를 넣어줄 것인지</param>
	/// <param name="cType">C언어에서 사용하는 자료형(문서보면 됨)</param>
	/// <param name="sqlType">DB에서 사용하는 자료형(문서보면 됨)</param>
	/// <param name="len">바인딩할 인자 크기</param>
	/// <param name="ptr">바인딩할 데이터 주소</param>
	/// <param name="index">가변 길이일 경우 그 길이, 가변 길이가 아니면 0</param>
	/// <returns>성공 유무</returns>
	bool BindParam(SQLUSMALLINT paramIndex, SQLSMALLINT cType, SQLSMALLINT sqlType, SQLULEN len, SQLPOINTER ptr, SQLLEN* index);

	/// <summary>
	/// 쿼리를 실행시키기 전에 결과물을 받아올 위치를 바인딩 해주는 함수
	/// </summary>
	/// <param name="columnIndex">선택할 테이블 열</param>
	/// <param name="cType">C언어에서 사용하는 자료형(문서보면 됨)</param>
	/// <param name="len">저장할 위치의 공간</param>
	/// <param name="value">저장 위치</param>
	/// <param name="index"></param>
	/// <returns>성공 유무</returns>
	bool BindCol(SQLUSMALLINT columnIndex, SQLSMALLINT cType, SQLLEN len, SQLPOINTER value, SQLLEN* index);

	//에러가 났을때 공용함수
	void HandleError(SQLRETURN ret);

private:
	//실제 커넥션을 담당하는 핸들
	SQLHDBC			_connection = SQL_NULL_HANDLE;
	//상태를 담당하는 핸들
	SQLHSTMT		_statement = SQL_NULL_HANDLE;
};

