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
			0 ~ 1024				: 32������ ����
			1024 ~ 2048		: 128������ ����
			2048 ~ 4096		: 256������ ����
		*/

		/*
			�� 48���� Ǯ�� �����
			0~1024�׷�(32byte����)
				32byte�� Ǯ, 64byte�� Ǯ, 96byte�� Ǯ ...
			1024~2048�׷�(64byte����)
				1152byte�� Ǯ, 1280byte�� Ǯ ...
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

	//O(1)�� �޸�Ǯ�� ������ ã�� ���� ���̺�
	MemoryPool* _poolTable[MAX_ALLOC_SIZE + 1];
};




template <typename Type, typename ...Args>
Type* xnew(Args&&... args)
{
	Type* memory = static_cast<Type*>(PoolAllocator::Alloc(sizeof(Type)));
	
	//placement new : �Ҵ�� �޸� ������ ������ ȣ��
	new(memory)Type(std::forward<Args>(args)...);
	//args �� ...�� ��� ���� ��ü�� �̷�

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