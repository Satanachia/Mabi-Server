#pragma once

#include "Types.h"

#pragma region TypeList : 타입리스트 구성하기

template <typename... T>
struct TypeList;

template <typename T, typename U>//기저사례
struct TypeList<T, U>
{
	using Head = T;
	using Tail = U;
};

template <typename T, typename... U>//재귀(?)
struct TypeList<T, U...>
{
	using Head = T;
	using Tail = TypeList<U...>;
};

#pragma endregion


#pragma region Length : 타입리스트의 길이 구하기

template <typename T>
struct Length;

template <>
struct Length<TypeList<>>//기저사례
{
	enum {value = 0};
};

template <typename T, typename... U>//재귀
struct Length<TypeList<T, U...>>
{
	enum { value = 1 + Length<TypeList<U...>>::value};
};

#pragma endregion


#pragma region TypeAt : 타입리스트에서 index번째 요소 구하기

template <typename TL, int32 index>
struct TypeAt;

template <typename Head, typename... Tail>//기저사례
struct TypeAt<TypeList<Head, Tail...>, 0>
{
	using Result = Head;
};

template <typename Head, typename... Tail, int32 index>//재귀
struct TypeAt<TypeList<Head, Tail...>, index>
{
	using Result = typename TypeAt<TypeList<Tail...>, index - 1>::Result;
};

#pragma endregion


#pragma region IndexOf : 특정요소가 타입리스트에서 몇번째인지 확인

template <typename TL, typename T>
struct IndexOf;

template <typename... Tail, typename T>
struct IndexOf<TypeList<T, Tail...>, T>//타입 리스트의 맨 앞에 요소를 지목한 경우(기저사례?)
{
	enum { value = 0 };
};

template <typename T>
struct IndexOf<TypeList<>, T>//타입리스트에 해당 요소가 없는 경우
{
	enum { value = -1 };
};

template<typename Head, typename... Tail, typename T>	//재귀
struct IndexOf<TypeList<Head, Tail...>, T>
{
private:
	enum { temp = IndexOf<TypeList<Tail...>, T>::value };	//재귀

public:
	enum { value = (-1 == temp) ? -1 : (temp + 1) };			//찾은 경우 값 반환, 못찾으면 -1반환
};

#pragma endregion


#pragma region Conversion : From에서 To로 캐스팅 가능 여부 판단

template <typename From, typename To>
class Conversion
{
private:
	using Small = __int8;
	using Big = __int32;

	static Small Test(const To&) { return 0; }	//컴파일러가 From이 To로 캐스팅 "가능"하다고 판단되면 여기로 오고 Small을 반환한다
	static Big Test(...) { return 0; }					//컴파일러가 From이 To로 캐스팅 "불가능"하다고 판단되면 여기로 오고 Big을 반환한다
	static From MakeFrom() { return 0; }			//1. From을 반환

public:
	enum
	{
		//From이 To로 캐스팅 가능하다면 Small이 반환된다
		exists = sizeof(Small) == sizeof(Test(MakeFrom()))
	};
};

#pragma endregion



#pragma region int2Type : 상수 자체를 구조체로 가지고 있는다.(TypeConversion::MakeTable을 할때 타입리스트 테이블을 채워주는 계산도 컴파일 타임에 실행하기 위함)

template<int32 v>
struct int2Type
{
	enum { value = v };
};

#pragma endregion


#pragma region TypeConversion : 타입리스트에 대한 테이블을 만들고 캐스팅 가능 여부를 TypeConversion가 생성될 때 계산

template<typename TL>
class TypeConversion
{
public:
	enum
	{
		length = Length<TL>::value
	};

	TypeConversion()
	{
		//[0][0]부터 [length - 1][length - 1]까지 타입리스트를 채워준다
		MakeTable(int2Type<0>(), int2Type<0>());
	}

	//타입리스트의 테이블을 채워주기(재귀함수)
	template<int32 i, int32 j>
	static void MakeTable(int2Type<i>, int2Type<j>)
	{
		using FromType = typename TypeAt<TL, i>::Result;	//타입리스트의 i번째 클래스
		using ToType = typename TypeAt<TL, j>::Result;		//타입리스트의 j번째 클래스

		//캐스팅이 가능한지 확인
		if (Conversion<const FromType*, const ToType*>::exists)
			s_convert[i][j] = true;
		else
			s_convert[i][j] = false;

		//2중 포문의 j인덱스 증가
		MakeTable(int2Type<i>(), int2Type<j + 1>());
	}

	//j가 length인 경우
	template <int32 i>
	static void MakeTable(int2Type<i>, int2Type<length>)
	{
		//j는 0으로 만들고 i인덱스 1 증가
		MakeTable(int2Type<i + 1>(), int2Type<0>());
	}

	//i가 length인 경우
	template <int32 j>
	static void MakeTable(int2Type<length>, int2Type<j>)
	{
		//아무런 작업도 하지 않음(기저사례)
	}
	
	//캐스팅 가능 여부 반환
	static inline bool CanConvert(int32 from, int32 to)
	{
		static TypeConversion conversion;
		return s_convert[from][to];
	}

public:
	//테이블의 크기또한 컴파일 타임에 결정
	static bool s_convert[length][length];	
};

template<typename TL>
bool TypeConversion<TL>::s_convert[length][length];

#pragma endregion



#pragma region TypeCast

//std::remove_pointer_t : 포인터 타입인 경우 포인터를 제거하고 클래스 형태만 남김
//_typeId : 타입캐스트를 사용하는 클래스들은 반드시 _typeId라는 멤버변수를 가지고 있음(타입리스트에서 자신의 클래스가 몇번째 인덱스에 위치했는지를 의미)

//생 포인터용 타입캐스트
template <typename To, typename From>
To TypeCast(From* ptr)
{
	if (nullptr == ptr)
		return nullptr;
	
	using TL = typename From::TL;
	if (TypeConversion<TL>::CanConvert(ptr->_typeId, IndexOf<TL, std::remove_pointer_t<To>>::value))
		return static_cast<To>(ptr);

	return nullptr;
}

//스마트 포인터용 타입캐스트
template <typename To, typename From>
std::shared_ptr<To> TypeCast(std::shared_ptr<From> ptr)
{
	if (nullptr == ptr)
		return nullptr;

	using TL = typename From::TL;
	if (TypeConversion<TL>::CanConvert(ptr->_typeId, IndexOf<TL, std::remove_pointer_t<To>>::value))
		return std::static_pointer_cast<To>(ptr);

	return nullptr;
}

//캐스팅은 하지 않고 캐스팅 가능 유무만 판단(생 포인터용)
template <typename To, typename From>
bool CanCast(From* ptr)
{
	if (nullptr == ptr)
		return false;

	using TL = typename From::TL;
	return TypeConversion<TL>::CanConvert(ptr->_typeId, IndexOf<TL, std::remove_pointer_t<To>>::value);
}

//캐스팅은 하지 않고 캐스팅 가능 유무만 판단(스마트 포인터용)
template <typename To, typename From>
bool CanCast(std::shared_ptr<From> ptr)
{
	if (nullptr == ptr)
		return false;

	using TL = typename From::TL;
	return TypeConversion<TL>::CanConvert(ptr->_typeId, IndexOf<TL, std::remove_pointer_t<To>>::value);
}

#pragma endregion

/*
 [DECLARE_TL]
	타입캐스트를 사용하기 위해서는 
	최상단 부모의 헤더에 
	using TL = TypeList<class Parent, class Child1, class Child2, class Child3>; 와 같이 
	타입리스트를 정의해주어야 한다.
	그리고 멤버변수로는 _typeId를 가지고 있는다.
	
[INIT_TL]
	타입캐스트를 사용하는 모든 클래스들은 생성자에서 
	자신이 타입리스트에 몇번째 인덱스인지 계산한다
*/

#define DECLARE_TL			using TL = TL;	int32 _typeId;
#define INIT_TL(Type)		_typeId = IndexOf<TL, Type>::value

/*
[타입캐스트 쓸때 주의점]
	Typecast<Child>(ptr)의 ptr의 헤더를 Typecast쓰는곳에서 include시키지 않으면
	TypeConversion::MakeTable에서 잘못된 값을 계산하게 됨
*/