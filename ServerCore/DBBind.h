#pragma once
#include "DBConnection.h"


template <int32 C>
struct FullBits { enum { value = (1 << (C - 1)) | FullBits<C - 1>::value }; };

template <>
struct FullBits<1> { enum { value = 1 }; };

template <>
struct FullBits<0> { enum { value = 0 }; };

/*
	//템플릿 재귀를 이용한 모든 비트 켜기
		FullBits<3> = (1 << 2) | FullBits<2>
		FullBits<3> = (1 << 2) | (1 << 1) | FullBits<1>
		FullBits<3> = (1 << 2) | (1 << 1) | 1
*/


/*----------------------------------------
							DBBind
----------------------------------------*/


/// <summary>
/// DB에 인자 혹은 결과값을 바인딩 하기 위한 래핑 클래스
/// </summary>
/// <typeparam name="ParamCount">쿼리를 실행시키기 전에 바인딩할 인자의 갯수</typeparam>
/// <typeparam name="ColumnCount">실행한 쿼리 결과를 Fetch로 실행할 때 결과물 갯수</typeparam>
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

	//문자열 바인드
	void BindParam(int32 idx, const WCHAR* value)
	{
		//문자열의 경우 내부에서 _paramIndex[idx]의 값(가변 길이 데이터의 크기)을 수정한다
		_dbConnection.BindParam(idx + 1, value, &_paramIndex[idx]);
		_paramFlag |= (1LL << idx);
	}

	//바이트 배열 바인드
	template <typename T, int32 N>
	void BindParam(int32 idx, T(&value)[N])
	{
		//내부에서 _paramIndex[idx]의 값(가변 길이 데이터의 크기)을 수정한다
		_dbConnection.BindParam(idx + 1, (const  BYTE*)value, size32(T) * N, &_paramIndex[idx]);
		_paramFlag |= (1LL << idx);
	}

	//포인터
	template <typename T>
	void BindParam(int32 idx, T* value, int32 N)
	{
		//내부에서 _paramIndex[idx]의 값(가변 길이 데이터의 크기)을 수정한다
		_dbConnection.BindParam(idx + 1, (const  BYTE*)value, size32(T) * N, &_paramIndex[idx]);
		_paramFlag |= (1LL << idx);
	}



	template <typename T>
	void BindCol(int32 idx, T& value)
	{
		_dbConnection.BindCol(idx + 1, &value, &_columnIndex[idx]);
		_columnFlag |= (1LL << idx);
	}

	//문자 배열
	template <int32 N>
	void BindCol(int32 idx, WCHAR(&value)[N])
	{
		//dbConnection::BindCol(인덱스, OUT 문자열 결과 반환, 버퍼 크기, OUT 반환된 결과 크기)
		_dbConnection.BindCol(idx + 1, value, N - 1, &_columnIndex[idx]);
		_columnFlag |= (1LL << idx);
	}

	//문자 포인터
	void BindCol(int32 idx, WCHAR* value, int32 len)
	{
		_dbConnection.BindCol(idx + 1, value, len - 1, &_columnIndex[idx]);
		_columnFlag |= (1LL << idx);
	}

	//배열
	template <typename T, int32 N>
	void BindCol(int32 idx, T(&value)[N])
	{
		_dbConnection.BindCol(idx + 1, value,size32(T) * N, &_columnIndex[idx]);
		_columnFlag |= (1LL << idx);
	}

protected:
	DBConnection&		_dbConnection;
	const WCHAR*		_query;

	//가변 길이일 경우 그 길이, 가변 길이가 아니면 0을 유지
	SQLLEN					_paramIndex[ParamCount > 0 ? ParamCount : 1];
	SQLLEN					_columnIndex[ColumnCount > 0 ? ColumnCount : 1];

	//비트 플래스를 이용해서 모든 인자들이 연결되었는지 확인한다
	uint64						_paramFlag;
	uint64						_columnFlag;
};

