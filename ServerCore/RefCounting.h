#pragma once


/*--------------------------------------
							RefCountable
--------------------------------------*/

class RefCountable
{
public:
	RefCountable()
		: _refCount(1)
	{

	}
	virtual ~RefCountable(){}

	RefCountable(const RefCountable& _Other) = delete;
	RefCountable(RefCountable&& _Other) noexcept = delete;
	RefCountable& operator=(const RefCountable& _Other) = delete;
	RefCountable& operator=(const RefCountable&& _Other) noexcept = delete;
	
	int32 GetRefCount() { return _refCount; }
	int32 AddRef() { return ++_refCount; }
	int32 ReleaseRef()
	{
		//���⿡ ���� ���� ������
		//delete�Ǵ� ���� this�����ʹ�
		//��۸������Ͱ� �Ǳ� ����
		int32 refCount = --_refCount;

		if (0 == refCount)
		{
			delete this;
		}
		return refCount;
	}
	
protected:

private:
	Atomic<int32> _refCount;
};

/*--------------------------------------
							Shared_ptr
--------------------------------------*/

template <typename T>
class TSharedPtr
{
public:
	TSharedPtr(){}
	TSharedPtr(T* ptr)
	{
		Set(ptr);
	}

	//����
	TSharedPtr(const TSharedPtr& rhs)
	{
		Set(rhs._ptr);
	}

	//�̵�
	TSharedPtr(TSharedPtr&& rhs)
	{
		_ptr = rhs._ptr;
		rhs._ptr = nullptr;
	}

	//��� ���� ����
	template<typename U>
	TSharedPtr(const TSharedPtr<U>& rhs)
	{
		//�ٿ� ĳ�����̴� �� ĳ�����̴� �ּҸ� T*�� ��ȯ��Ű�µ�
		//�̰� ������ ����? ��� ���谡 �ƴϸ� ��¿����?
		// 
		//�̰� ���ø��̴ٺ��ϱ� TŬ������ UŬ������ ������ �����̴�
		//�׷��� TŬ���������� UŬ������ private��������� _ptr�� ������ �� ����
		// 
		//�θ��ڽ� ������ ��쿣 friend ó���ϸ� ������ ����ȴ�
		Set(static_cast<T*>(rhs._ptr));
	}

	~TSharedPtr()
	{
		Release();
	}

public:
	//���翬����
	TSharedPtr& operator=(const TSharedPtr& rhs)
	{
		if (_ptr != rhs._ptr)
		{
			Release();
			Set(rhs._ptr);
		}

		return *this;
	}


	//���翬����
	TSharedPtr& operator=(TSharedPtr&& rhs)
	{
		Release();
		_ptr = rhs._ptr;
		rhs._ptr = nullptr;
		return *this;
	}

	bool operator==(const TSharedPtr& rhs) const
	{
		return (_ptr == rhs._ptr);
	}
	bool operator==(T* ptr) const
	{
		return (_ptr == ptr);
	}
	bool operator!=(const TSharedPtr& rhs) const
	{
		return (_ptr != rhs._ptr);
	}
	bool operator!=(T* ptr) const
	{
		return (_ptr != ptr);
	}
	bool operator<(const TSharedPtr& rhs) const
	{
		return (_ptr < rhs._ptr);
	}
	T* operator*()
	{
		return _ptr;
	}
	const T* operator*()const
	{
		return _ptr;
	}
	operator T* () const
	{
		return _ptr;
	}
	T* operator->()
	{
		return _ptr;
	}
	const T* operator->()const
	{
		return _ptr;
	}



	bool IsNull() const
	{
		return (nullptr == _ptr);
	}

	

private:
	inline void Set(T* ptr)
	{
		_ptr = ptr;
		if (nullptr != ptr)
			ptr->AddRef();
	}

	inline void Release()
	{
		if (nullptr == _ptr)
			return;

		_ptr->ReleaseRef();
		_ptr = nullptr;
	}

private:
	T* _ptr = nullptr;
};