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
		//여기에 값을 받은 이유는
		//delete되는 순간 this포인터는
		//댕글링포인터가 되기 때문
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

	//복사
	TSharedPtr(const TSharedPtr& rhs)
	{
		Set(rhs._ptr);
	}

	//이동
	TSharedPtr(TSharedPtr&& rhs)
	{
		_ptr = rhs._ptr;
		rhs._ptr = nullptr;
	}

	//상속 관계 복사
	template<typename U>
	TSharedPtr(const TSharedPtr<U>& rhs)
	{
		//다운 캐스팅이던 업 캐스팅이던 주소를 T*로 변환시키는데
		//이게 문제가 없나? 상속 관계가 아니면 어쩔려구?
		// 
		//이게 템플릿이다보니까 T클래스는 U클래스와 별개의 공간이다
		//그래서 T클래스에서는 U클래스의 private멤버변수인 _ptr에 접근할 수 없다
		// 
		//부모자식 관계의 경우엔 friend 처리하면 컴파일 통과된대
		Set(static_cast<T*>(rhs._ptr));
	}

	~TSharedPtr()
	{
		Release();
	}

public:
	//복사연산자
	TSharedPtr& operator=(const TSharedPtr& rhs)
	{
		if (_ptr != rhs._ptr)
		{
			Release();
			Set(rhs._ptr);
		}

		return *this;
	}


	//복사연산자
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