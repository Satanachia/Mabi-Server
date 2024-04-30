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
		Recv�� �ϳ��� ���ǿ����� ���� ����Ǳ� ������
		Ȯ���� Lock�� �ʿ������
		Send�� ���,
		������ ���� Send�� �� �Ϸ�Ǳ� ����  �� Send�� ���� ���� �ִ�.
		(������ �ʿ��� ����ϰ� Send�� �����µ�,
		���� �ٸ� �����忡�� Recv�� �� �� ��ε�ĳ��Ʈ�� �ϴ� ���)

		����
		https://social.msdn.microsoft.com/Forums/en-US/1f85557c-15f6-42b3-a8f7-1630b4099bb6/wsasend-from-multiple-thread-gt-safe?forum=vcgeneral
		�� �ۿ��� ������ WSASend�� ��Ƽ�����忡�� �������� �ʴ�

		�ٵ� �Ʒ� _sendQueue�� �������� �ٲٰ�
		�� ���� ���ټ� ����

		???????????
	*/

	if (false == IsConnected())
		return;

	//�� ��� ������ ���̱� ���� ���� ������ ���� ����
	bool regissterSend = false;

	{
		WRITE_LOCK;

		//RegisterSend -> ProcessSend �� �Ϸ���� �ʾҴٸ�
		//_sendQueue�� �����͸� �ױ⸸ �ϰ� ����
		//(������ ������ ť�� ��Ƶα�)
		_sendQueue.push(sendBuffer);

		//�� ���ǿ� ���ؼ� �ϳ��� �����常 RegisterSend���� ����
		if (false == _sendRegisterd.exchange(true))
		{
			regissterSend = true;
		}
	}

	//�� ���ǿ� ���ؼ� �ϳ��� �����常 RegisterSend����
	if(true == regissterSend)
		RegisterSend();
}

bool Session::Connect()
{
	return RegisterConnect();
}

void Session::Disconnect(const WCHAR* cause)
{
	//exchange : false�� �־��ְ� ������ ���� �����
	//�̹� _connected�� false���ٸ� Disconnect�� �� �������� ����
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

	//Ŭ���̾�Ʈ�� Connect�� ȣ���� ��쿡��
	if (ServiceType::Client != GetService()->GetServiceType())
		return false;

	//�ּ� ����
	if (false == SocketUtils::SetReuseAddress(_socket, true))
		return false;

	//���ϰ� ���� �ּ� ����
	//SocketUtils::ConnectEx�� ������ �� �� �Լ��� �� ������� �־�� ��
	if (false == SocketUtils::BindAnyAddress(_socket, 0/*�˾Ƽ�*/))
		return false;

	_connectEvent.Init();
	_connectEvent.owner = shared_from_this();

	DWORD numOfBytes = 0;

	//�� Ŭ���̾�Ʈ�� ����� �ּ�
	//Ŭ���̾�Ʈ�� ���񽺿����� NetAddress�� SocketAddr�� ����(����)�� �ּҸ� �ǹ���
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
		// Disconnect���� ����Ѽ����� 
		// AcceptEx�� ConnectEx���� ������ �� �ְ� ����
		//�ٵ� ������ ������ Ǯ�������� ����
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
	//�� ������ refCount 1 ����
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
	
	//��¥ ���� �߻�
	HandleError(errorCode);
	//�� ������ refCount 1 ����
	_recvEvent.owner = nullptr;
}

void Session::RegisterSend()
{
	if (false == IsConnected())
		return;

	_sendEvent.Init();
	//�� ������ refCount 1 ����
	_sendEvent.owner = shared_from_this();


	//���� �����͸� SendEvent�� ���
	{
		//ť ������ �� ��� �ߴµ�,
		//���߿��� �ۿ� �ִ� ���� Ǯ���� Ȯ���� ����
		WRITE_LOCK;

		//������ ���� ������
		int32 writeSize = 0;

		//������ ť -> SendEvent�� ���ͷ� �̵�
		while (false == _sendQueue.empty())
		{
			SendBufferRef sendBuffer = _sendQueue.front();
			_sendQueue.pop();

			//���� ������ ũ�� ����
			writeSize += sendBuffer->WriteSize();

			//TODO : �ʹ� ���� �����͸� ���������� while�� ����������
			//���߿� ��Ŷ ���� ������ �׷�
			//[2����Ʈ : ��Ŷ ������][...]
			//2����Ʈ�� ǥ���� �� �ִ� ũ���� �ִ� ������

			_sendEvent.sendBuffers.push_back(sendBuffer);
		}
	}

	//Scatter-Gather (������ִ� �����͸� �ѹ��� ��Ƽ� ����)
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
	
	//���� ������ �ִ� ���
	HandleError(errorCode);

	//refCount ����
	_sendEvent.owner = nullptr;
	_sendEvent.sendBuffers.clear();

	//HandleError���� ������ ���������
	//�׷��� false�� ����ϰ� ��������
	_sendRegisterd.store(false);
}

void Session::ProcessConnect()
{
	/*
		�� �Լ��� ���� ���� ���� ������
		ProcessAccept������ ȣ��ǰ� �ְ�
		Ŭ���� RegisterConnect -> CP -> Dispatch�� ���ؼ��� ȣ��ǰ� ����
		������ �Ʒ� �ڵ���� �� ��Ȳ ��� ũ�� ������ �ȵ�
	*/

	_connectEvent.owner = nullptr;

	_connected.store(true);

	//���� ���
	GetService()->AddSession(GetSessionRef());

	//�������̵� �� ������ �� �Լ� ȣ��
	OnConnected();

	//Recv ����
	RegisterRecv();
}

void Session::ProcessDisconnect()
{
	_disconnectEvent.owner = nullptr;

	//������ �ʿ��� �������̵� �� �Լ�
	OnDisconnected();

	//���񽺿��� �� ������ ����(���ο��� refCount ����)
	GetService()->ReleaseSession(GetSessionRef());
}

void Session::ProcessRecv(int32 numOfBytes)
{
	/*
		Recv�� �� ���ǿ� �ִ� �Ѱ��� ����ϰ� �Ǳ� ������,
		��Ƽ������ ��Ȳ�̿��� ������ ����
		(������ ��� ������ �������̱� ������)
	*/

	//�� ������ refCount 1 ����
	_recvEvent.owner = nullptr;

	//������ ����
	if (0 == numOfBytes)
	{
		Disconnect(L"Recv Zero");
		return;
	}

	//recv������ write������ Ŀ�� �̵�
	if (false == _recvBuffer.OnWrite(numOfBytes))
	{
		Disconnect(L"OnWrite Overflow");
		return;
	}

	//���±��� recv���ۿ� ���� ������ũ��
	int32 dataSize = _recvBuffer.DataSize();

	//������ �� �Լ� ȣ��
	int32 processLen = OnRecv(_recvBuffer.ReadPos(), dataSize);

	//������ �ʿ��� ó���� ������ ũ�� ����ó��
	if ((processLen < 0) || (dataSize < processLen))
	{
		Disconnect(L"ProcessLen Error");
		return;
	}

	//recv���� ����ó��
	if (false == _recvBuffer.OnRead(processLen))
	{
		Disconnect(L"OnRead Overflow");
		return;
	}
	
	//���� ����
	_recvBuffer.Clean();

	//���� ���
	RegisterRecv();
}

void Session::ProcessSend(int32 numOfBytes)
{
	//refCount����
	_sendEvent.owner = nullptr;
	_sendEvent.sendBuffers.clear();

	//���� ����
	if (0 == numOfBytes)
	{
		Disconnect(L"Send Zero");
		return;
	}

	//������ �� ȣ��
	OnSend(numOfBytes);

	WRITE_LOCK;

	if (true == _sendQueue.empty())
	{
		//�������� �ٽ� RegisterSend����
		_sendRegisterd.store(false);
	}
	else
	{
		//�����ִ� �����Ϳ� ���� ::WSASend
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
		//���߿� �α׸� ��� ���� �����忡�� ����� ����
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

		//��Ŷ ����� �Ľ��� �� �ִ��� Ȯ��
		if (dataSize < sizeof(PacketHeader))
			break;

		//����
		PacketHeader header = *reinterpret_cast<PacketHeader*>(&buffer[processLen]);

		//��Ŷ ũ�� �Ľ� �������� Ȯ��
		if (dataSize < header.size)
			break;

		//��Ŷ ����
		OnRecvPacket(&buffer[processLen], header.size);
		processLen += header.size;
	}

	return processLen;
}