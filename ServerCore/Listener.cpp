#include "pch.h"
#include "Listener.h"
#include "SocketUtils.h"
#include "IocpEvent.h"
#include "Session.h"
#include "Service.h"

/*----------------------------------
					Listener
----------------------------------*/

Listener::Listener()
{

}

Listener::~Listener()
{
	SocketUtils::Close(_socket);

	for (AcceptEvent* acceptEvent : _acceptEvents)
	{


		xdelete(acceptEvent);
	}
}

bool Listener::StartAccept(ServerServiceRef service)
{
	_service = service;
	if (nullptr == _service)
		return false;

	//소켓 생성
	_socket = SocketUtils::CreateSocket();
	if (INVALID_SOCKET == _socket)
		return false;

	//iocp에 리스너 소켓 등록
	if (false == _service->GetIocpCore()->Register(shared_from_this()))
		return false;

	//옵션 설정1
	if (false == SocketUtils::SetReuseAddress(_socket, true))
		return false;

	//옵션 설정2
	if (false == SocketUtils::SetLinger(_socket, 0, 0))
		return false;

	//리슨소켓과 서버주소 연동
	if (false == SocketUtils::Bind(_socket, _service->GetNetAddress()))
		return false;

	//클라 입장 시작(+ 클라 입장 대기열 생성)
	if (false == SocketUtils::Listen(_socket))
		return false;

	//이벤트 갯수가 적으면
	//일부 클라는 서버에 접속 못할 수 있음
	const int32 acceptCount = _service->GetMaxSessionCount();
	for (size_t i = 0; i < acceptCount; ++i)
	{
		//클라가 하는지 감시하는 이벤트 등록
		AcceptEvent* acceptEvent = xnew<AcceptEvent>();

		//acceptEvent 의 주인은 Listener
		acceptEvent->owner = shared_from_this();
		_acceptEvents.push_back(acceptEvent);
		RegisterAccept(acceptEvent);
	}
	
	return true;
}

void Listener::CloseSocket()
{
	SocketUtils::Close(_socket);
}

HANDLE Listener::GetHandle()
{
	return reinterpret_cast<HANDLE>(_socket);
}

void Listener::Dispatch(IocpEvent* iocpEvent, int32 numOfBytes)
{
	//IocpCore의 Dispatch를 통해 여기로 오게 된다

	ASSERT_CRASH(iocpEvent->eventType == EventType::Accept);

	//AcceptEvent로 변환 및 처리
	AcceptEvent* acceptEvent = static_cast<AcceptEvent*>(iocpEvent);
	ProcessAccept(acceptEvent);
}

void Listener::RegisterAccept(AcceptEvent* acceptEvent)
{
	//클라 접속할 때 연동될 세션 미리 만들면서 CP에 등록
	SessionRef clientSession = _service->CreateSession();

	//acceptEvent에 연동
	acceptEvent->Init();
	acceptEvent->clientSession = clientSession;

	//비동기 Accept 실행
	DWORD bytesReceived;
	bool result = SocketUtils::AcceptEx(
		_socket,														//리슨소켓
		clientSession->GetSocket(),						//클라이언트 소켓이 될 Accept소켓 등록
		clientSession->_recvBuffer.WritePos(), 0,	//리시브 버퍼와 시작위치
		sizeof(SOCKADDR_IN) + 16,						//튜토리얼에 이렇게 나와있음
		sizeof(SOCKADDR_IN) + 16,						//튜토리얼에 이렇게 나와있음
		OUT & bytesReceived,
		static_cast<LPOVERLAPPED>(acceptEvent));
	//마지막 인자를 AcceptEvent로 넣음
	//AcceptEvent는 session포인터를 가지고 있음(나중에 ProcessAccept에서 사용됨)

	if(false == result)
	{
		const int32 errorCode = WSAGetLastError();
		if (WSA_IO_PENDING != errorCode)
		{
			//진짜 문제 발생

			//일단 다시 Accept걸어준다
			RegisterAccept(acceptEvent);
		}
	}
}

void Listener::ProcessAccept(AcceptEvent* acceptEvent)
{
	//클라이언트 세션
	SessionRef clientSession = acceptEvent->clientSession;
	/*
		GetQueuedCompletionStatus를 빠져나오면서
		CP가 acceptEvent의 _session에 클라이언트 세션을 채워주었음
		클라이언트 세션을 등록하는 부분은 Listener::RegisterAccept의 SocketUtils::AcceptEx
	*/

	
	if (false == SocketUtils::SetUpdateAcceptSocket(clientSession->GetSocket(), _socket))
	{
		//실패를 했어도 예약은 다시 걸어주어야 한다
		RegisterAccept(acceptEvent);
		return;
	}

	//소켓으로 주소 받아오기
	SOCKADDR_IN sockAddress;
	int32 sizeOfSockAddr = sizeof(sockAddress);
	if (SOCKET_ERROR == ::getpeername(clientSession->GetSocket(), OUT reinterpret_cast<SOCKADDR*>(&sockAddress), &sizeOfSockAddr))
	{
		RegisterAccept(acceptEvent);
		return;
	}

	clientSession->SetNetAddress(NetAddress(sockAddress));
	
	clientSession->ProcessConnect();

	//다시 Accept예약하기
	RegisterAccept(acceptEvent);
}
