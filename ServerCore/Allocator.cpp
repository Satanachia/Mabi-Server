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
	//size�� PAGE_SIZE �ٱ��� � ����������
	const int64 pageCount = (size + PAGE_SIZE - 1) / PAGE_SIZE;

	//�����÷ο� ������ ���� �Ҵ�� �޸𸮸� ���������� ������ ������
	//���� ������ ��ġ
	//(����÷ο�� ũ���ð� �� �������� ����÷ο�� �� �� �Ͼ�ϱ�)
	const int64 dataOffset = (pageCount * PAGE_SIZE) - size;

	void* baseAddress = ::VirtualAlloc(NULL, pageCount * PAGE_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	//'������ ������' ����Ʈ ������ �ϱ� ����
	// int8*�� ��ȯ��Ű��
	//dataOffset�� ����
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
