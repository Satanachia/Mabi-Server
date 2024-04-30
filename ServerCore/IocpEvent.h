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
	OVERLAPPED를 상속하는 방식으로 했는데
	이때 가상함수 사용하면 메모리의 첫 주소가 OVERLAPPED가 아니게 된다
	그러니 이 부분을 조심하자
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

	//IocpObjet의 use after free를 막기위해
	//IocpEvent가 shared_ptr로 들고 있는다
	IocpObjectRef	owner;
};


/*----------------------------------
					ConnectEvent
---------------------------------*/

//세션 내부에서 멤버변수로 관리되고 있음
class ConnectEvent : public IocpEvent
{
public:
	ConnectEvent() : IocpEvent(EventType::Connect){}
};

/*----------------------------------
					DisconnectEvent
---------------------------------*/

//세션 내부에서 멤버변수로 관리되고 있음
class DisconnectEvent : public IocpEvent
{
public:
	DisconnectEvent() : IocpEvent(EventType::Disconnect) {}
};


/*----------------------------------
					AcceptEvent
---------------------------------*/

//리스너에서 사용중
class AcceptEvent : public IocpEvent
{
public:
	AcceptEvent() : IocpEvent(EventType::Accept) {}

public:
	/*
		SocketUtils::AcceptEx을 호출할 때
		인자로 클라이언트 소켓을 같이 받아준다.
		(이전에는 ::accept 함수에서 반환값으로 나왔다)

		이 클라이언트 소켓을
		GetQueuedCompletionStatus를 호출하는 스레드에서
		사용하기 위해 AcceptEvent가 내부에 포인터로 들고 있는다
		(AcceptEvent의 최상위 부모는 OVERLAPPED로, CP를 이동할 수 있다)
	*/
	SessionRef clientSession = nullptr;
};


/*----------------------------------
					RecvEvent
---------------------------------*/

//세션 내부에서 멤버변수로 관리되고 있음
class RecvEvent : public IocpEvent
{
public:
	RecvEvent() : IocpEvent(EventType::Recv) {}
};


/*----------------------------------
					SendEvent
---------------------------------*/

//세션 내부에서 멤버변수로 관리되고 있음
class SendEvent : public IocpEvent
{
public:
	SendEvent() : IocpEvent(EventType::Send) {}

	//SendBuffer가 Session::ProcessSend 완료때까지
	//  refCount 유지 시키기 위해 가지고 있음
	Vector<SendBufferRef> sendBuffers;
};
