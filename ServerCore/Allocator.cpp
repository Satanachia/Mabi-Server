#include "pch.h"
#include "Allocator.h"
#include "Memory.h"

/*--------------------------------------------------
											BaseAllocator
--------------------------------------------------*/

void* BaseAllocator::Alloc(int32 size)
{
	return ::malloc(size);
}

void BaseAllocator::Release(void* ptr) 
{
	::free(ptr);
}

/*--------------------------------------------------
											StompAllocator
--------------------------------------------------*/

void* StompAllocator::Alloc(int32 size)
{
	//size가 PAGE_SIZE 바구니 몇개 나눠지는지
	const int64 pageCount = (size + PAGE_SIZE - 1) / PAGE_SIZE;

	//오버플로우 방지를 위해 할당된 메모리를 페이지에서 오른쪽 정렬함
	//실제 데이터 위치
	//(언더플로우는 크래시가 안 나겠지만 언더플로우는 잘 안 일어나니까)
	const int64 dataOffset = (pageCount * PAGE_SIZE) - size;

	void* baseAddress = ::VirtualAlloc(NULL, pageCount * PAGE_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	//'포인터 연산을' 바이트 단위로 하기 위해
	// int8*로 변환시키고
	//dataOffset을 더함
	return static_cast<void*>(static_cast<int8*>(baseAddress) + dataOffset);
}

void StompAllocator::Release(void* ptr)
{
	const int64 address = reinterpret_cast<int64>(ptr);
	const int64 baseAddress = address - (address % PAGE_SIZE);
	::VirtualFree(reinterpret_cast<void*>(baseAddress), 0, MEM_RELEASE);
}

/*--------------------------------------------------
											PoolAllocator
--------------------------------------------------*/

void* PoolAllocator::Alloc(int32 size)
{
	return GMemory->Allocate(size);
}

void PoolAllocator::Release(void* ptr)
{
	GMemory->Release(ptr);
}
