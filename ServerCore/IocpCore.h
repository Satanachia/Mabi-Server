#pragma once

/*----------------------------------
					IocpObject
----------------------------------*/

//Iocp에 Key로 등록할 수 있는 오브젝트
class IocpObject : public std::enable_shared_from_this<IocpObject>
{
public:
	virtual HANDLE GetHandle() abstract;

	//GetQueuedCompletionStatus를 한 뒤 어떻게 처리할 지에 대한 함수
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) abstract;
};


/*----------------------------------
					NetAddress
----------------------------------*/

class IocpCore
{
public:
	IocpCore();
	~IocpCore();

	IocpCore(const IocpCore& _Other) = delete;
	IocpCore(IocpCore&& _Other) noexcept = delete;
	IocpCore& operator=(const IocpCore& _Other) = delete;
	IocpCore& operator=(const IocpCore&& _Other) noexcept = delete;

	HANDLE	GetHandle() { return _iocpHandle; }

	//소켓을 iocp에 등록하는 함수
	bool			Register(IocpObjectRef iocpObject);
	//스레드들이 cp에 일감이 있는지 확인하는 함수
	bool			Dispatch(uint32 timeoutMs = INFINITE);

	

private:
	HANDLE _iocpHandle;
};
