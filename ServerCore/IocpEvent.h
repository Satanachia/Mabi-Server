#pragma once

class Session;

/*----------------------------------
					EventType
----------------------------------*/

enum class EventType : uint8
{
	Connect,
	Disconnect,
	Accept,
	Recv,
	Send,
};

/*----------------------------------
					IocpEvent
----------------------------------*/

/*
	OVERLAPPED�� ����ϴ� ������� �ߴµ�
	�̶� �����Լ� ����ϸ� �޸��� ù �ּҰ� OVERLAPPED�� �ƴϰ� �ȴ�
	�׷��� �� �κ��� ��������
*/
class IocpEvent : public OVERLAPPED
{
public:
	IocpEvent(EventType type);
	~IocpEvent();

	IocpEvent(const IocpEvent& _Other) = delete;
	IocpEvent(IocpEvent&& _Other) noexcept = delete;
	IocpEvent& operator=(const IocpEvent& _Other) = delete;
	IocpEvent& operator=(const IocpEvent&& _Other) noexcept = delete;

	void	Init();

public:
	EventType			eventType;

	//IocpObjet�� use after free�� ��������
	//IocpEvent�� shared_ptr�� ��� �ִ´�
	IocpObjectRef	owner;
};


/*----------------------------------
					ConnectEvent
---------------------------------*/

//���� ���ο��� ��������� �����ǰ� ����
class ConnectEvent : public IocpEvent
{
public:
	ConnectEvent() : IocpEvent(EventType::Connect){}
};

/*----------------------------------
					DisconnectEvent
---------------------------------*/

//���� ���ο��� ��������� �����ǰ� ����
class DisconnectEvent : public IocpEvent
{
public:
	DisconnectEvent() : IocpEvent(EventType::Disconnect) {}
};


/*----------------------------------
					AcceptEvent
---------------------------------*/

//�����ʿ��� �����
class AcceptEvent : public IocpEvent
{
public:
	AcceptEvent() : IocpEvent(EventType::Accept) {}

public:
	/*
		SocketUtils::AcceptEx�� ȣ���� ��
		���ڷ� Ŭ���̾�Ʈ ������ ���� �޾��ش�.
		(�������� ::accept �Լ����� ��ȯ������ ���Դ�)

		�� Ŭ���̾�Ʈ ������
		GetQueuedCompletionStatus�� ȣ���ϴ� �����忡��
		����ϱ� ���� AcceptEvent�� ���ο� �����ͷ� ��� �ִ´�
		(AcceptEvent�� �ֻ��� �θ�� OVERLAPPED��, CP�� �̵��� �� �ִ�)
	*/
	SessionRef clientSession = nullptr;
};


/*----------------------------------
					RecvEvent
---------------------------------*/

//���� ���ο��� ��������� �����ǰ� ����
class RecvEvent : public IocpEvent
{
public:
	RecvEvent() : IocpEvent(EventType::Recv) {}
};


/*----------------------------------
					SendEvent
---------------------------------*/

//���� ���ο��� ��������� �����ǰ� ����
class SendEvent : public IocpEvent
{
public:
	SendEvent() : IocpEvent(EventType::Send) {}

	//SendBuffer�� Session::ProcessSend �Ϸᶧ����
	//  refCount ���� ��Ű�� ���� ������ ����
	Vector<SendBufferRef> sendBuffers;
};
