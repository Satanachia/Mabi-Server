#pragma once
#include "DBConnection.h"


template <int32 C>
struct FullBits { enum { value = (1 << (C - 1)) | FullBits<C - 1>::value }; };

template <>
struct FullBits<1> { enum { value = 1 }; };

template <>
struct FullBits<0> { enum { value = 0 }; };

/*
	//���ø� ��͸� �̿��� ��� ��Ʈ �ѱ�
		FullBits<3> = (1 << 2) | FullBits<2>
		FullBits<3> = (1 << 2) | (1 << 1) | FullBits<1>
		FullBits<3> = (1 << 2) | (1 << 1) | 1
*/


/*----------------------------------------
							DBBind
----------------------------------------*/


/// <summary>
/// DB�� ���� Ȥ�� ������� ���ε� �ϱ� ���� ���� Ŭ����
/// </summary>
/// <typeparam name="ParamCount">������ �����Ű�� ���� ���ε��� ������ ����</typeparam>
/// <typeparam name="ColumnCount">������ ���� ����� Fetch�� ������ �� ����� ����</typeparam>
template <int32 ParamCount, int32 ColumnCount>
class DBBind
{
public:
	DBBind(DBConnection& dbConnection, const WCHAR* query)
		:_dbConnection(dbConnection), _query(query)
	{
		::memset(_paramIndex, 0, sizeof(_paramIndex));
		::memset(_columnIndex, 0, sizeof(_columnIndex));
		_paramFlag = 0;
		_columnFlag = 0;
		dbConnection.Unbind();
	}
	~DBBind(){}

	DBBind(const DBBind& _Other) = delete;
	DBBind(DBBind&& _Other) noexcept = delete;
	DBBind& operator=(const DBBind& _Other) = delete;
	DBBind& operator=(const DBBind&& _Other) noexcept = delete;

	bool Validate()
	{
		uint64 paramFullBit = FullBits<ParamCount>::value;
		uint64 columnFullBit = FullBits<ColumnCount>::value;
		return _paramFlag == paramFullBit && _columnFlag == columnFullBit;
	}

	bool Execute()
	{
		ASSERT_CRASH(Validate());
		return _dbConnection.Execute(_query);
	}

	bool Fetch()
	{
		return _dbConnection.Fetch();
	}

public:
	template <typename T>
	void BindParam(int32 idx, T& value)
	{
		_dbConnection.BindParam(idx + 1, &value, &_paramIndex[idx]);
		_paramFlag |= (1LL << idx);
	}

	//���ڿ� ���ε�
	void BindParam(int32 idx, const WCHAR* value)
	{
		//���ڿ��� ��� ���ο��� _paramIndex[idx]�� ��(���� ���� �������� ũ��)�� �����Ѵ�
		_dbConnection.BindParam(idx + 1, value, &_paramIndex[idx]);
		_paramFlag |= (1LL << idx);
	}

	//����Ʈ �迭 ���ε�
	template <typename T, int32 N>
	void BindParam(int32 idx, T(&value)[N])
	{
		//���ο��� _paramIndex[idx]�� ��(���� ���� �������� ũ��)�� �����Ѵ�
		_dbConnection.BindParam(idx + 1, (const  BYTE*)value, size32(T) * N, &_paramIndex[idx]);
		_paramFlag |= (1LL << idx);
	}

	//������
	template <typename T>
	void BindParam(int32 idx, T* value, int32 N)
	{
		//���ο��� _paramIndex[idx]�� ��(���� ���� �������� ũ��)�� �����Ѵ�
		_dbConnection.BindParam(idx + 1, (const  BYTE*)value, size32(T) * N, &_paramIndex[idx]);
		_paramFlag |= (1LL << idx);
	}



	template <typename T>
	void BindCol(int32 idx, T& value)
	{
		_dbConnection.BindCol(idx + 1, &value, &_columnIndex[idx]);
		_columnFlag |= (1LL << idx);
	}

	//���� �迭
	template <int32 N>
	void BindCol(int32 idx, WCHAR(&value)[N])
	{
		//dbConnection::BindCol(�ε���, OUT ���ڿ� ��� ��ȯ, ���� ũ��, OUT ��ȯ�� ��� ũ��)
		_dbConnection.BindCol(idx + 1, value, N - 1, &_columnIndex[idx]);
		_columnFlag |= (1LL << idx);
	}

	//���� ������
	void BindCol(int32 idx, WCHAR* value, int32 len)
	{
		_dbConnection.BindCol(idx + 1, value, len - 1, &_columnIndex[idx]);
		_columnFlag |= (1LL << idx);
	}

	//�迭
	template <typename T, int32 N>
	void BindCol(int32 idx, T(&value)[N])
	{
		_dbConnection.BindCol(idx + 1, value,size32(T) * N, &_columnIndex[idx]);
		_columnFlag |= (1LL << idx);
	}

protected:
	DBConnection&		_dbConnection;
	const WCHAR*		_query;

	//���� ������ ��� �� ����, ���� ���̰� �ƴϸ� 0�� ����
	SQLLEN					_paramIndex[ParamCount > 0 ? ParamCount : 1];
	SQLLEN					_columnIndex[ColumnCount > 0 ? ColumnCount : 1];

	//��Ʈ �÷����� �̿��ؼ� ��� ���ڵ��� ����Ǿ����� Ȯ���Ѵ�
	uint64						_paramFlag;
	uint64						_columnFlag;
};

