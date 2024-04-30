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
	//[MemoryHeader][할당할 메모리]
	const int32 allocSize = size + sizeof(MemoryHeader);

#ifdef _STOMP
	header = reinterpret_cast<MemoryHeader*>(StompAllocator::Alloc(allocSize));
#else
	if (MAX_ALLOC_SIZE < allocSize)
	{
		//메모리 풀링 최대 크기를 벗어나면 일반 할당
		header = reinterpret_cast<MemoryHeader*>(::_aligned_malloc(allocSize, SLIST_ALIGNMENT));
	}
	else
	{
		header = _poolTable[allocSize]->Pop();
	}
#endif

	//할당된 메모리에 MemoryHeader생성자 호출
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
		//메모리 풀링 최대 크기를 벗어난 경우
		::_aligned_free(header);
	}
	else
	{
		//풀에 반납
		_poolTable[allocSize]->Push(header);
	}
#endif
	
}