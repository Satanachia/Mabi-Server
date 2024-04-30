#pragma once

enum
{
	SLIST_ALIGNMENT = 16
};

/*--------------------------------------------
								MemoryHeader
--------------------------------------------*/


/*
	�޸� ������
	[MemoryHeader][data]
	�̷� ������ ������
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
		// �̹� �Ҵ�� �޸𸮿��� MemoryHeader�����ڸ� ȣ��
		new(header)MemoryHeader(size);
		//�ǳ��� ���� ����� �ǳ� �� �κ��� ����Ű�� ��ȯ
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

	//����� �޸� �ݳ�
	void Push(MemoryHeader* ptr);
	//�޸� Ǯ���� �ϳ� ���
	MemoryHeader* Pop();

protected:

private:
	SLIST_HEADER _header;

	//�ڽ��� ����ϰ� �ִ� Ǯ�� ������
	//(������ ������� �޸� Ǯ�� ������� ����)
	int32 _allocSize = 0;
	
	//�ۿ��� ������� �޸� ����
	std::atomic<int32> _useCount = 0;
	//Ǯ�� �����ϴ� �޸� ����
	std::atomic<int32> _reserveCount = 0;
};

