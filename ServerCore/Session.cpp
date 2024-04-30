#include "pch.h"
#include "Session.h"
#include "SocketUtils.h"
#include "Service.h"

/*----------------------------------
					Session
----------------------------------*/

Session::Session()
	:_recvBuffer(BUFFER_SIZE)
{
	_socket = SocketUtils::CreateSocket();
}

Session::~Session()
{
	SocketUtils::Close(_socket);
}

void Session::Send(SendBufferRef sendBuffer)
{
	/*
		Recv는 하나의 세션에서만 각자 실행되기 떄문에
		확실히 Lock이 필요없지만
		Send의 경우,
		이전에 보낸 Send가 다 완료되기 전에  또 Send를 보낼 수도 있다.
		(컨텐츠 쪽에서 평범하게 Send를 보내는데,
		하필 다른 스레드에서 Recv를 한 후 브로드캐스트를 하는 경우)

		또한
		https://social.msdn.microsoft.com/Forums/en-US/1f85557c-15f6-42b3-a8f7-1630b4099bb6/wsasend-from-multiple-thread-gt-safe?forum=vcgeneral
		위 글에서 따르면 WSASend은 멀티스레드에서 안전하지 않다

		근데 아래 _sendQueue는 락프리로 바꾸고
		이 락을 없앨수 있음

		???????????
	*/

	if (false == IsConnected())
		return;

	//락 잡는 범위를 줄이기 위해 스택 영역에 변수 설정
	bool regissterSend = false;

	{
		WRITE_LOCK;

		//RegisterSend -> ProcessSend 가 완료되지 않았다면
		//_sendQueue에 데이터를 쌓기만 하고 종료
		//(전송할 데이터 큐에 모아두기)
		_sendQueue.push(sendBuffer);

		//한 세션에 대해서 하나의 스레드만 RegisterSend동작 가능
		if (false == _sendRegisterd.exchange(true))
		{
			regissterSend = true;
		}
	}

	//한 세션에 대해서 하나의 스레드만 RegisterSend동작
	if(true == regissterSend)
		RegisterSend();
}

bool Session::Connect()
{
	return RegisterConnect();
}

void Session::Disconnect(const WCHAR* cause)
{
	//exchange : false를 넣어주고 기존의 값을 뱉어줌
	//이미 _connected가 false였다면 Disconnect를 더 실행하지 않음
	if (false == _connected.exchange(false))
		return;

	RegisterDisconnect();
}

HANDLE Session::GetHandle()
{
	return reinterpret_cast<HANDLE>(_socket);
}

void Session::Dispatch(IocpEvent* iocpEvent, int32 numOfBytes)
{
	switch (iocpEvent->eventType)
	{
	case EventType::Connect:
		ProcessConnect();
		break;
	case EventType::Recv:
		ProcessRecv(numOfBytes);
		break;
	case EventType::Send:
		ProcessSend(numOfBytes);
		break;
	case EventType::Disconnect:
		ProcessDisconnect();
		break;
	default:
		break;
	}
}

bool Session::RegisterConnect()
{
	if (true == IsConnected())
		return false;

	//클라이언트가 Connect를 호출한 경우에만
	if (ServiceType::Client != GetService()->GetServiceType())
		return false;

	//주소 재사용
	if (false == SocketUtils::SetReuseAddress(_socket, true))
		return false;

	//소켓과 서버 주소 연동
	//SocketUtils::ConnectEx로 연결할 땐 이 함수를 꼭 실행시켜 주어야 함
	if (false == SocketUtils::BindAnyAddress(_socket, 0/*알아서*/))
		return false;

	_connectEvent.Init();
	_connectEvent.owner = shared_from_this();

	DWORD numOfBytes = 0;

	//이 클라이언트가 연결될 주소
	//클라이언트의 서비스에서는 NetAddress의 SocketAddr는 상대방(서버)쪽 주소를 의미함
	SOCKADDR_IN sockAddr = GetService()->GetNetAddress().GetSocketAddr();


	if (TRUE == SocketUtils::ConnectEx(_socket,
		reinterpret_cast<SOCKADDR*>(&sockAddr),
		sizeof(sockAddr), nullptr, 0,
		&numOfBytes, &_connectEvent))
	{
		return true;
	}

	int32 errorCode = ::WSAGetLastError();
	if (WSA_IO_PENDING == errorCode)
		return true;

	_connectEvent.owner = nullptr;
	return false;

}

bool Session::RegisterDisconnect()
{
	_disconnectEvent.Init();
	_disconnectEvent.owner = shared_from_this();

	//TF_REUSE_SOCKET
		// Disconnect에서 사용한소켓을 
		// AcceptEx나 ConnectEx에서 재사용할 수 있게 해줌
		//근데 실제로 소켓을 풀링하지는 않음
	if (TRUE == SocketUtils::DisconnectEx(_socket, &_disconnectEvent, TF_REUSE_SOCKET, 0))
		return true;

	int32 errorCode = ::WSAGetLastError();
	if (WSA_IO_PENDING == errorCode)
		return true;

	_disconnectEvent.owner = nullptr;
	return false;
}

void Session::RegisterRecv()
{
	if (false == IsConnected())
		return;

	_recvEvent.Init();
	//이 세션의 refCount 1 증가
	_recvEvent.owner = shared_from_this();

	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer.WritePos());
	wsaBuf.len = _recvBuffer.FreeSize();

	DWORD numOfBytes = 0;
	DWORD flags = 0;

	if (SOCKET_ERROR != ::WSARecv(
		_socket, &wsaBuf, 1, 
		OUT &numOfBytes, OUT &flags, 
		&_recvEvent, nullptr))
	{
		return;
	}

	int32 errorCode = ::WSAGetLastError();
	if (WSA_IO_PENDING == errorCode)
		return;
	
	//진짜 에러 발생
	HandleError(errorCode);
	//이 세션의 refCount 1 감소
	_recvEvent.owner = nullptr;
}

void Session::RegisterSend()
{
	if (false == IsConnected())
		return;

	_sendEvent.Init();
	//이 세션의 refCount 1 증가
	_sendEvent.owner = shared_from_this();


	//보낼 데이터를 SendEvent에 등록
	{
		//큐 떄문에 락 잡긴 했는데,
		//나중에은 밖에 있는 락을 풀어줄 확률이 높음
		WRITE_LOCK;

		//누적된 전송 데이터
		int32 writeSize = 0;

		//세션의 큐 -> SendEvent의 벡터로 이동
		while (false == _sendQueue.empty())
		{
			SendBufferRef sendBuffer = _sendQueue.front();
			_sendQueue.pop();

			//전송 데이터 크기 누적
			writeSize += sendBuffer->WriteSize();

			//TODO : 너무 많은 데이터를 전송했을때 while문 빠져나오기
			//나중에 패킷 구조 때문에 그럼
			//[2바이트 : 패킷 사이즈][...]
			//2바이트가 표현할 수 있는 크기의 최대 값때문

			_sendEvent.sendBuffers.push_back(sendBuffer);
		}
	}

	//Scatter-Gather (흩어져있는 데이터를 한번에 모아서 전송)
	Vector<WSABUF> wasBufs;
	wasBufs.reserve(_sendEvent.sendBuffers.size());
	for (SendBufferRef sendBuffer : _sendEvent.sendBuffers)
	{
		WSABUF wsaBuf;
		wsaBuf.buf = reinterpret_cast<char*>(sendBuffer->Buffer());
		wsaBuf.len = static_cast<LONG>(sendBuffer->WriteSize());
		wasBufs.push_back(wsaBuf);
	}

	DWORD numOfBytes = 0;
	if (SOCKET_ERROR != ::WSASend(
		_socket, wasBufs.data(), 
		static_cast<DWORD>(wasBufs.size()),
		OUT &numOfBytes, 0, &_sendEvent, nullptr))
	{
		return;
	}

	int32 errorCode = ::WSAGetLastError();
	if (WSA_IO_PENDING == errorCode)
		return;
	
	//정말 문제가 있는 경우
	HandleError(errorCode);

	//refCount 감소
	_sendEvent.owner = nullptr;
	_sendEvent.sendBuffers.clear();

	//HandleError에서 연결이 끊기겠지만
	//그래도 false로 깔끔하게 정리하자
	_sendRegisterd.store(false);
}

void Session::ProcessConnect()
{
	/*
		이 함수는 지금 서버 리슨 소켓의
		ProcessAccept에서도 호출되고 있고
		클라의 RegisterConnect -> CP -> Dispatch를 통해서도 호출되고 있음
		하지만 아래 코드들은 두 상황 모두 크게 문제는 안됨
	*/

	_connectEvent.owner = nullptr;

	_connected.store(true);

	//세션 등록
	GetService()->AddSession(GetSessionRef());

	//오버라이딩 된 컨텐츠 쪽 함수 호출
	OnConnected();

	//Recv 예약
	RegisterRecv();
}

void Session::ProcessDisconnect()
{
	_disconnectEvent.owner = nullptr;

	//컨텐츠 쪽에서 오버라이딩 될 함수
	OnDisconnected();

	//서비스에서 이 세션을 제거(내부에서 refCount 감소)
	GetService()->ReleaseSession(GetSessionRef());
}

void Session::ProcessRecv(int32 numOfBytes)
{
	/*
		Recv는 한 세션에 최대 한개만 사용하게 되기 때문에,
		멀티스레드 상황이여도 문제가 없다
		(세션이 모두 독립된 데이터이기 때문임)
	*/

	//이 세션의 refCount 1 감소
	_recvEvent.owner = nullptr;

	//연결이 끊김
	if (0 == numOfBytes)
	{
		Disconnect(L"Recv Zero");
		return;
	}

	//recv버퍼의 write버퍼의 커서 이동
	if (false == _recvBuffer.OnWrite(numOfBytes))
	{
		Disconnect(L"OnWrite Overflow");
		return;
	}

	//여태까지 recv버퍼에 쌓인 데이터크기
	int32 dataSize = _recvBuffer.DataSize();

	//컨텐츠 쪽 함수 호출
	int32 processLen = OnRecv(_recvBuffer.ReadPos(), dataSize);

	//컨텐츠 쪽에서 처리한 데이터 크기 예외처리
	if ((processLen < 0) || (dataSize < processLen))
	{
		Disconnect(L"ProcessLen Error");
		return;
	}

	//recv버퍼 예외처리
	if (false == _recvBuffer.OnRead(processLen))
	{
		Disconnect(L"OnRead Overflow");
		return;
	}
	
	//버퍼 정리
	_recvBuffer.Clean();

	//수신 등록
	RegisterRecv();
}

void Session::ProcessSend(int32 numOfBytes)
{
	//refCount감소
	_sendEvent.owner = nullptr;
	_sendEvent.sendBuffers.clear();

	//전송 실패
	if (0 == numOfBytes)
	{
		Disconnect(L"Send Zero");
		return;
	}

	//컨텐츠 쪽 호출
	OnSend(numOfBytes);

	WRITE_LOCK;

	if (true == _sendQueue.empty())
	{
		//이제부터 다시 RegisterSend가능
		_sendRegisterd.store(false);
	}
	else
	{
		//남아있는 데이터에 대해 ::WSASend
		RegisterSend();
	}
}

void Session::HandleError(int32 errorCode)
{
	switch (errorCode)
	{
	case WSAECONNRESET:
	case WSAECONNABORTED:
		Disconnect(L"HandleError");
		break;
	default:
		//나중에 로그를 찍는 전문 스레드에서 출력할 예정
		std::cout << "Handle Error : " << errorCode << std::endl;
		break;
	}
}


/*----------------------------------
					PacketSession
----------------------------------*/

PacketSession::PacketSession()
{

}

PacketSession::~PacketSession()
{

}

int32 PacketSession::OnRecv(BYTE* buffer, int32 len)
{
	int32 processLen = 0;

	while (true)
	{
		int32 dataSize = (len - processLen);

		//패킷 헤더를 파싱할 수 있는지 확인
		if (dataSize < sizeof(PacketHeader))
			break;

		//복사
		PacketHeader header = *reinterpret_cast<PacketHeader*>(&buffer[processLen]);

		//패킷 크기 파싱 가능한지 확인
		if (dataSize < header.size)
			break;

		//패킷 조립
		OnRecvPacket(&buffer[processLen], header.size);
		processLen += header.size;
	}

	return processLen;
}