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

	//���� ����
	_socket = SocketUtils::CreateSocket();
	if (INVALID_SOCKET == _socket)
		return false;

	//iocp�� ������ ���� ���
	if (false == _service->GetIocpCore()->Register(shared_from_this()))
		return false;

	//�ɼ� ����1
	if (false == SocketUtils::SetReuseAddress(_socket, true))
		return false;

	//�ɼ� ����2
	if (false == SocketUtils::SetLinger(_socket, 0, 0))
		return false;

	//�������ϰ� �����ּ� ����
	if (false == SocketUtils::Bind(_socket, _service->GetNetAddress()))
		return false;

	//Ŭ�� ���� ����(+ Ŭ�� ���� ��⿭ ����)
	if (false == SocketUtils::Listen(_socket))
		return false;

	//�̺�Ʈ ������ ������
	//�Ϻ� Ŭ��� ������ ���� ���� �� ����
	const int32 acceptCount = _service->GetMaxSessionCount();
	for (size_t i = 0; i < acceptCount; ++i)
	{
		//Ŭ�� �ϴ��� �����ϴ� �̺�Ʈ ���
		AcceptEvent* acceptEvent = xnew<AcceptEvent>();

		//acceptEvent �� ������ Listener
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
	//IocpCore�� Dispatch�� ���� ����� ���� �ȴ�

	ASSERT_CRASH(iocpEvent->eventType == EventType::Accept);

	//AcceptEvent�� ��ȯ �� ó��
	AcceptEvent* acceptEvent = static_cast<AcceptEvent*>(iocpEvent);
	ProcessAccept(acceptEvent);
}

void Listener::RegisterAccept(AcceptEvent* acceptEvent)
{
	//Ŭ�� ������ �� ������ ���� �̸� ����鼭 CP�� ���
	SessionRef clientSession = _service->CreateSession();

	//acceptEvent�� ����
	acceptEvent->Init();
	acceptEvent->clientSession = clientSession;

	//�񵿱� Accept ����
	DWORD bytesReceived;
	bool result = SocketUtils::AcceptEx(
		_socket,														//��������
		clientSession->GetSocket(),						//Ŭ���̾�Ʈ ������ �� Accept���� ���
		clientSession->_recvBuffer.WritePos(), 0,	//���ú� ���ۿ� ������ġ
		sizeof(SOCKADDR_IN) + 16,						//Ʃ�丮�� �̷��� ��������
		sizeof(SOCKADDR_IN) + 16,						//Ʃ�丮�� �̷��� ��������
		OUT & bytesReceived,
		static_cast<LPOVERLAPPED>(acceptEvent));
	//������ ���ڸ� AcceptEvent�� ����
	//AcceptEvent�� session�����͸� ������ ����(���߿� ProcessAccept���� ����)

	if(false == result)
	{
		const int32 errorCode = WSAGetLastError();
		if (WSA_IO_PENDING != errorCode)
		{
			//��¥ ���� �߻�

			//�ϴ� �ٽ� Accept�ɾ��ش�
			RegisterAccept(acceptEvent);
		}
	}
}

void Listener::ProcessAccept(AcceptEvent* acceptEvent)
{
	//Ŭ���̾�Ʈ ����
	SessionRef clientSession = acceptEvent->clientSession;
	/*
		GetQueuedCompletionStatus�� ���������鼭
		CP�� acceptEvent�� _session�� Ŭ���̾�Ʈ ������ ä���־���
		Ŭ���̾�Ʈ ������ ����ϴ� �κ��� Listener::RegisterAccept�� SocketUtils::AcceptEx
	*/

	
	if (false == SocketUtils::SetUpdateAcceptSocket(clientSession->GetSocket(), _socket))
	{
		//���и� �߾ ������ �ٽ� �ɾ��־�� �Ѵ�
		RegisterAccept(acceptEvent);
		return;
	}

	//�������� �ּ� �޾ƿ���
	SOCKADDR_IN sockAddress;
	int32 sizeOfSockAddr = sizeof(sockAddress);
	if (SOCKET_ERROR == ::getpeername(clientSession->GetSocket(), OUT reinterpret_cast<SOCKADDR*>(&sockAddress), &sizeOfSockAddr))
	{
		RegisterAccept(acceptEvent);
		return;
	}

	clientSession->SetNetAddress(NetAddress(sockAddress));
	
	clientSession->ProcessConnect();

	//�ٽ� Accept�����ϱ�
	RegisterAccept(acceptEvent);
}
