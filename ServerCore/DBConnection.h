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

	//���� ������ �����ϴ� �Լ�
	bool Execute(const WCHAR* query);

	//SELECT �� ó�� ����� �޾ƿ� �� ����ϴ� �Լ�
	bool Fetch();

	//���� ���� ��������
	int32 GetRowCount();

	//BindParam/BindCol�� ���������� ���ε�� ������ ����(�ʱ�ȭ)
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
	/// ������ �����Ű�� ���� ���ڸ� ���ε����ִ� �Լ�
	/// </summary>
	/// <param name="paramIndex">���° ���ڷ� �����͸� �־��� ������</param>
	/// <param name="cType">C���� ����ϴ� �ڷ���(�������� ��)</param>
	/// <param name="sqlType">DB���� ����ϴ� �ڷ���(�������� ��)</param>
	/// <param name="len">���ε��� ���� ũ��</param>
	/// <param name="ptr">���ε��� ������ �ּ�</param>
	/// <param name="index">���� ������ ��� �� ����, ���� ���̰� �ƴϸ� 0</param>
	/// <returns>���� ����</returns>
	bool BindParam(SQLUSMALLINT paramIndex, SQLSMALLINT cType, SQLSMALLINT sqlType, SQLULEN len, SQLPOINTER ptr, SQLLEN* index);

	/// <summary>
	/// ������ �����Ű�� ���� ������� �޾ƿ� ��ġ�� ���ε� ���ִ� �Լ�
	/// </summary>
	/// <param name="columnIndex">������ ���̺� ��</param>
	/// <param name="cType">C���� ����ϴ� �ڷ���(�������� ��)</param>
	/// <param name="len">������ ��ġ�� ����</param>
	/// <param name="value">���� ��ġ</param>
	/// <param name="index"></param>
	/// <returns>���� ����</returns>
	bool BindCol(SQLUSMALLINT columnIndex, SQLSMALLINT cType, SQLLEN len, SQLPOINTER value, SQLLEN* index);

	//������ ������ �����Լ�
	void HandleError(SQLRETURN ret);

private:
	//���� Ŀ�ؼ��� ����ϴ� �ڵ�
	SQLHDBC			_connection = SQL_NULL_HANDLE;
	//���¸� ����ϴ� �ڵ�
	SQLHSTMT		_statement = SQL_NULL_HANDLE;
};

