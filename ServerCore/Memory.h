#pragma once
#include "Allocator.h"

class MemoryPool;

/*--------------------------------------------
								Memory
--------------------------------------------*/

class Memory
{
	enum
	{
		/*
			0 ~ 1024				: 32단위로 나눔
			1024 ~ 2048		: 128단위로 나눔
			2048 ~ 4096		: 256단위로 나눔
		*/

		/*
			총 48개의 풀이 생긴다
			0~1024그룹(32byte단위)
				32byte의 풀, 64byte의 풀, 96byte의 풀 ...
			1024~2048그룹(64byte단위)
				1152byte의 풀, 1280byte의 풀 ...
			...
		*/
		POOL_COUNT = (1024 / 32) + (1024 / 128) + (2048 / 256),
		MAX_ALLOC_SIZE = 4096
	};

public:
	Memory();
	~Memory();

	void* Allocate(int32 size);
	void Release(void* ptr);

private:
	std::vector<MemoryPool*> _pools;

	//O(1)로 메모리풀을 빠르게 찾기 위한 테이블
	MemoryPool* _poolTable[MAX_ALLOC_SIZE + 1];
};




template <typename Type, typename ...Args>
Type* xnew(Args&&... args)
{
	Type* memory = static_cast<Type*>(PoolAllocator::Alloc(sizeof(Type)));
	
	//placement new : 할당된 메모리 위에서 생성자 호출
	new(memory)Type(std::forward<Args>(args)...);
	//args 뒤 ...은 사용 문법 자체가 이럼

	return memory;
}

template <typename Type>
void xdelete(Type* obj)
{
	obj->~Type();
	PoolAllocator::Release(obj);
}

template <typename Type, typename ...Args>
std::shared_ptr<Type> MakeShared(Args&&... args)
{
	return std::shared_ptr<Type>{xnew<Type>(std::forward<Args>(args)...), xdelete<Type>};
}