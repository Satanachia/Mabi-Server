#pragma once

#include "Types.h"

#pragma region TypeList : Ÿ�Ը���Ʈ �����ϱ�

template <typename... T>
struct TypeList;

template <typename T, typename U>//�������
struct TypeList<T, U>
{
	using Head = T;
	using Tail = U;
};

template <typename T, typename... U>//���(?)
struct TypeList<T, U...>
{
	using Head = T;
	using Tail = TypeList<U...>;
};

#pragma endregion


#pragma region Length : Ÿ�Ը���Ʈ�� ���� ���ϱ�

template <typename T>
struct Length;

template <>
struct Length<TypeList<>>//�������
{
	enum {value = 0};
};

template <typename T, typename... U>//���
struct Length<TypeList<T, U...>>
{
	enum { value = 1 + Length<TypeList<U...>>::value};
};

#pragma endregion


#pragma region TypeAt : Ÿ�Ը���Ʈ���� index��° ��� ���ϱ�

template <typename TL, int32 index>
struct TypeAt;

template <typename Head, typename... Tail>//�������
struct TypeAt<TypeList<Head, Tail...>, 0>
{
	using Result = Head;
};

template <typename Head, typename... Tail, int32 index>//���
struct TypeAt<TypeList<Head, Tail...>, index>
{
	using Result = typename TypeAt<TypeList<Tail...>, index - 1>::Result;
};

#pragma endregion


#pragma region IndexOf : Ư����Ұ� Ÿ�Ը���Ʈ���� ���°���� Ȯ��

template <typename TL, typename T>
struct IndexOf;

template <typename... Tail, typename T>
struct IndexOf<TypeList<T, Tail...>, T>//Ÿ�� ����Ʈ�� �� �տ� ��Ҹ� ������ ���(�������?)
{
	enum { value = 0 };
};

template <typename T>
struct IndexOf<TypeList<>, T>//Ÿ�Ը���Ʈ�� �ش� ��Ұ� ���� ���
{
	enum { value = -1 };
};

template<typename Head, typename... Tail, typename T>	//���
struct IndexOf<TypeList<Head, Tail...>, T>
{
private:
	enum { temp = IndexOf<TypeList<Tail...>, T>::value };	//���

public:
	enum { value = (-1 == temp) ? -1 : (temp + 1) };			//ã�� ��� �� ��ȯ, ��ã���� -1��ȯ
};

#pragma endregion


#pragma region Conversion : From���� To�� ĳ���� ���� ���� �Ǵ�

template <typename From, typename To>
class Conversion
{
private:
	using Small = __int8;
	using Big = __int32;

	static Small Test(const To&) { return 0; }	//�����Ϸ��� From�� To�� ĳ���� "����"�ϴٰ� �ǴܵǸ� ����� ���� Small�� ��ȯ�Ѵ�
	static Big Test(...) { return 0; }					//�����Ϸ��� From�� To�� ĳ���� "�Ұ���"�ϴٰ� �ǴܵǸ� ����� ���� Big�� ��ȯ�Ѵ�
	static From MakeFrom() { return 0; }			//1. From�� ��ȯ

public:
	enum
	{
		//From�� To�� ĳ���� �����ϴٸ� Small�� ��ȯ�ȴ�
		exists = sizeof(Small) == sizeof(Test(MakeFrom()))
	};
};

#pragma endregion



#pragma region int2Type : ��� ��ü�� ����ü�� ������ �ִ´�.(TypeConversion::MakeTable�� �Ҷ� Ÿ�Ը���Ʈ ���̺��� ä���ִ� ��굵 ������ Ÿ�ӿ� �����ϱ� ����)

template<int32 v>
struct int2Type
{
	enum { value = v };
};

#pragma endregion


#pragma region TypeConversion : Ÿ�Ը���Ʈ�� ���� ���̺��� ����� ĳ���� ���� ���θ� TypeConversion�� ������ �� ���

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
		//[0][0]���� [length - 1][length - 1]���� Ÿ�Ը���Ʈ�� ä���ش�
		MakeTable(int2Type<0>(), int2Type<0>());
	}

	//Ÿ�Ը���Ʈ�� ���̺��� ä���ֱ�(����Լ�)
	template<int32 i, int32 j>
	static void MakeTable(int2Type<i>, int2Type<j>)
	{
		using FromType = typename TypeAt<TL, i>::Result;	//Ÿ�Ը���Ʈ�� i��° Ŭ����
		using ToType = typename TypeAt<TL, j>::Result;		//Ÿ�Ը���Ʈ�� j��° Ŭ����

		//ĳ������ �������� Ȯ��
		if (Conversion<const FromType*, const ToType*>::exists)
			s_convert[i][j] = true;
		else
			s_convert[i][j] = false;

		//2�� ������ j�ε��� ����
		MakeTable(int2Type<i>(), int2Type<j + 1>());
	}

	//j�� length�� ���
	template <int32 i>
	static void MakeTable(int2Type<i>, int2Type<length>)
	{
		//j�� 0���� ����� i�ε��� 1 ����
		MakeTable(int2Type<i + 1>(), int2Type<0>());
	}

	//i�� length�� ���
	template <int32 j>
	static void MakeTable(int2Type<length>, int2Type<j>)
	{
		//�ƹ��� �۾��� ���� ����(�������)
	}
	
	//ĳ���� ���� ���� ��ȯ
	static inline bool CanConvert(int32 from, int32 to)
	{
		static TypeConversion conversion;
		return s_convert[from][to];
	}

public:
	//���̺��� ũ����� ������ Ÿ�ӿ� ����
	static bool s_convert[length][length];	
};

template<typename TL>
bool TypeConversion<TL>::s_convert[length][length];

#pragma endregion



#pragma region TypeCast

//std::remove_pointer_t : ������ Ÿ���� ��� �����͸� �����ϰ� Ŭ���� ���¸� ����
//_typeId : Ÿ��ĳ��Ʈ�� ����ϴ� Ŭ�������� �ݵ�� _typeId��� ��������� ������ ����(Ÿ�Ը���Ʈ���� �ڽ��� Ŭ������ ���° �ε����� ��ġ�ߴ����� �ǹ�)

//�� �����Ϳ� Ÿ��ĳ��Ʈ
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

//����Ʈ �����Ϳ� Ÿ��ĳ��Ʈ
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

//ĳ������ ���� �ʰ� ĳ���� ���� ������ �Ǵ�(�� �����Ϳ�)
template <typename To, typename From>
bool CanCast(From* ptr)
{
	if (nullptr == ptr)
		return false;

	using TL = typename From::TL;
	return TypeConversion<TL>::CanConvert(ptr->_typeId, IndexOf<TL, std::remove_pointer_t<To>>::value);
}

//ĳ������ ���� �ʰ� ĳ���� ���� ������ �Ǵ�(����Ʈ �����Ϳ�)
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
	Ÿ��ĳ��Ʈ�� ����ϱ� ���ؼ��� 
	�ֻ�� �θ��� ����� 
	using TL = TypeList<class Parent, class Child1, class Child2, class Child3>; �� ���� 
	Ÿ�Ը���Ʈ�� �������־�� �Ѵ�.
	�׸��� ��������δ� _typeId�� ������ �ִ´�.
	
[INIT_TL]
	Ÿ��ĳ��Ʈ�� ����ϴ� ��� Ŭ�������� �����ڿ��� 
	�ڽ��� Ÿ�Ը���Ʈ�� ���° �ε������� ����Ѵ�
*/

#define DECLARE_TL			using TL = TL;	int32 _typeId;
#define INIT_TL(Type)		_typeId = IndexOf<TL, Type>::value

/*
[Ÿ��ĳ��Ʈ ���� ������]
	Typecast<Child>(ptr)�� ptr�� ����� Typecast���°����� include��Ű�� ������
	TypeConversion::MakeTable���� �߸��� ���� ����ϰ� ��
*/