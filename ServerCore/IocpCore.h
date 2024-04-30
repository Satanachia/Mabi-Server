#pragma once

/*----------------------------------
					IocpObject
----------------------------------*/

//Iocp�� Key�� ����� �� �ִ� ������Ʈ
class IocpObject : public std::enable_shared_from_this<IocpObject>
{
public:
	virtual HANDLE GetHandle() abstract;

	//GetQueuedCompletionStatus�� �� �� ��� ó���� ���� ���� �Լ�
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

	//������ iocp�� ����ϴ� �Լ�
	bool			Register(IocpObjectRef iocpObject);
	//��������� cp�� �ϰ��� �ִ��� Ȯ���ϴ� �Լ�
	bool			Dispatch(uint32 timeoutMs = INFINITE);

	

private:
	HANDLE _iocpHandle;
};
