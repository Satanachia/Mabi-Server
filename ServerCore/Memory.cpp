#include "pch.h"
#include "Memory.h"
#include "MemoryPool.h"

/*--------------------------------------------
								Memory
--------------------------------------------*/

Memory::Memory()
{
	int32 size = 0;
	int32 tableIndex = 0;
	_pools.reserve(POOL_COUNT);

	for (size = 32; size <= 1024; size += 32)
	{
		MemoryPool* pool = new MemoryPool(size);
		_pools.push_back(pool);

		while (tableIndex <= size)
		{
			_poolTable[tableIndex] = pool;
			++tableIndex;
		}
	}

	for (; size <= 2048; size += 128)
	{
		MemoryPool* pool = new MemoryPool(size);
		_pools.push_back(pool);

		while (tableIndex <= size)
		{
			_poolTable[tableIndex] = pool;
			++tableIndex;
		}
	}

	for (; size <= 4096; size += 256)
	{
		MemoryPool* pool = new MemoryPool(size);
		_pools.push_back(pool);

		while (tableIndex <= size)
		{
			_poolTable[tableIndex] = pool;
			++tableIndex;
		}
	}
}

Memory::~Memory()
{
	for (MemoryPool* pool : _pools)
	{
		delete pool;
	}

	_pools.clear();
}

void* Memory::Allocate(int32 size) 
{
	MemoryHeader* header = nullptr;
	//[MemoryHeader][�Ҵ��� �޸�]
	const int32 allocSize = size + sizeof(MemoryHeader);

#ifdef _STOMP
	header = reinterpret_cast<MemoryHeader*>(StompAllocator::Alloc(allocSize));
#else
	if (MAX_ALLOC_SIZE < allocSize)
	{
		//�޸� Ǯ�� �ִ� ũ�⸦ ����� �Ϲ� �Ҵ�
		header = reinterpret_cast<MemoryHeader*>(::_aligned_malloc(allocSize, SLIST_ALIGNMENT));
	}
	else
	{
		header = _poolTable[allocSize]->Pop();
	}
#endif

	//�Ҵ�� �޸𸮿� MemoryHeader������ ȣ��
	return MemoryHeader::AttachHeader(header, allocSize);
}

void Memory::Release(void* ptr)
{
	MemoryHeader* header = MemoryHeader::DetachHeader(ptr);

	const int32 allocSize = header->allocSize;
	ASSERT_CRASH(0 < allocSize);

#ifdef _STOMP
	StompAllocator::Release(header);
#else
	if (MAX_ALLOC_SIZE < allocSize)
	{
		//�޸� Ǯ�� �ִ� ũ�⸦ ��� ���
		::_aligned_free(header);
	}
	else
	{
		//Ǯ�� �ݳ�
		_poolTable[allocSize]->Push(header);
	}
#endif
	
}