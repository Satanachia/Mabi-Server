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

//���� �ü������ �޸� �Ҵ�/��ü�� ��Ź�ϱ� ������
//�޸� ���� ������ ������ ������ �� ����
// 
//���� new/delete�� �ѹ��� ū ������ �Ҵ�޾� Reserveó���� �����صΰ� �����
// �޸� ������ �ؾ� �� ���� ��쿡 ����
// �ٷ� �޸� ������ ���� �ʰ� ���������� ����ȭ�� ��(�Ѳ����� �����Ѵٴ���...)
//�̷��� ũ���ð� �Ͼ�� �ʰ� �޸� ������ �߻��Ѵ�
//
// ���� �� ����� �޸� ���� ���ϰ�(���� ����ȭ), ���� ���޸𸮿� �Ҵ��� ���� �ϱ� ������
//�翬�� ����� ȯ�濡���� ����� ���̰� ������� ���X, Only �޸� ������ Ȯ���ϱ� ���� ������
class StompAllocator
{
	//������ ����(4kb -> 4096byte)
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

	//���ڷ� ��� �����ؼ� �Ҵ��� ���� ���� ���´�
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