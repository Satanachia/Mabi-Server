#pragma once

enum
{
	SLIST_ALIGNMENT = 16
};

/*--------------------------------------------
								MemoryHeader
--------------------------------------------*/


/*
	메모리 관리를
	[MemoryHeader][data]
	이런 식으로 관리함
*/
DECLSPEC_ALIGN(SLIST_ALIGNMENT)
struct MemoryHeader : public SLIST_ENTRY
{
	MemoryHeader(int32 size)
		:allocSize(size)
	{
		
	}

	static void* AttachHeader(MemoryHeader* header, int32 size)
	{
		//placement new
		// 이미 할당된 메모리에서 MemoryHeader생성자를 호출
		new(header)MemoryHeader(size);
		//건네줄 때는 헤더를 건너 뛴 부분을 가르키게 반환
		return reinterpret_cast<void*>(++header);
	}

	static MemoryHeader* DetachHeader(void* ptr)
	{
		MemoryHeader* header = reinterpret_cast<MemoryHeader*>(ptr) - 1;
		return header;
	}
	
	int32 allocSize;
};

/*--------------------------------------------
								MemoryPool
--------------------------------------------*/

DECLSPEC_ALIGN(SLIST_ALIGNMENT)
class MemoryPool
{
public:
	MemoryPool(int32 allocSize);
	~MemoryPool();

	MemoryPool(const MemoryPool& _Other) = delete;
	MemoryPool(MemoryPool&& _Other) noexcept = delete;
	MemoryPool& operator=(const MemoryPool& _Other) = delete;
	MemoryPool& operator=(const MemoryPool&& _Other) noexcept = delete;

	//사용한 메모리 반납
	void Push(MemoryHeader* ptr);
	//메모리 풀에서 하나 사용
	MemoryHeader* Pop();

protected:

private:
	SLIST_HEADER _header;

	//자신이 담당하고 있는 풀의 사이즈
	//(사이즈 종류대로 메모리 풀을 만들어줄 예정)
	int32 _allocSize = 0;
	
	//밖에서 사용중인 메모리 갯수
	std::atomic<int32> _useCount = 0;
	//풀에 존재하는 메모리 갯수
	std::atomic<int32> _reserveCount = 0;
};

