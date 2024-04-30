#pragma once

/*--------------------------------------------------
											BaseAllocator
--------------------------------------------------*/
class BaseAllocator
{
public:
	static void* Alloc(int32 size);
	static void Release(void* ptr);
};

/*--------------------------------------------------
											StompAllocator
--------------------------------------------------*/

//직접 운영체제에게 메모리 할당/해체를 부탁하기 때문에
//메모리 오염 문제를 사전에 방지할 수 있음
// 
//기존 new/delete는 한번에 큰 영역을 할당받아 Reserve처리로 예약해두고 사용함
// 메모리 해제를 해야 할 때는 경우에 따라
// 바로 메모리 해제를 하지 않고 내부적으로 최적화를 함(한꺼번에 삭제한다던가...)
//이러면 크래시가 일어나지 않고 메모리 오염이 발생한다
//
// 보면 이 방식은 메모리 낭비도 심하고(내부 단편화), 실제 힙메모리에 할당을 자주 하기 때문에
//당연히 디버그 환경에서만 사용할 것이고 릴리즈에선 사용X, Only 메모리 오염을 확인하기 위한 디버깅용
class StompAllocator
{
	//페이지 단위(4kb -> 4096byte)
	enum {PAGE_SIZE = 0x1000};

public:
	static void* Alloc(int32 size);
	static void Release(void* ptr);
};


/*--------------------------------------------------
											PoolAllocator
--------------------------------------------------*/

class PoolAllocator
{
public:
	static void* Alloc(int32 size);
	static void Release(void* ptr);
};



/*--------------------------------------------------
											STL Allocator
--------------------------------------------------*/

template <typename T>
class StlAllocator
{
public:
	using value_type = T;

	StlAllocator(){}

	template <typename Other>
	StlAllocator(const StlAllocator<Other>&){}

	//인자로 몇개를 연속해서 할당할 지에 대해 들어온다
	T* allocate(size_t count)
	{
		const int32 size = static_cast<int32>(count * sizeof(T));
		return static_cast<T*>(PoolAllocator::Alloc(size));
	}

	void deallocate(T* ptr, size_t count)
	{
		PoolAllocator::Release(ptr);
	}
};