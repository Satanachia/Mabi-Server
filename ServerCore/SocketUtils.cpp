#include "pch.h"
#include "SocketUtils.h"

/*----------------------------------
					SocketUtils
----------------------------------*/

LPFN_CONNECTEX			SocketUtils::ConnectEx		= nullptr;
LPFN_DISCONNECTEX		SocketUtils::DisconnectEx	= nullptr;
LPFN_ACCEPTEX				SocketUtils::AcceptEx			= nullptr;

void SocketUtils::Init()
{
	WSADATA wsaData;
	ASSERT_CRASH(0 == ::WSAStartup(MAKEWORD(2, 2), OUT & wsaData));

	//::connect, ::disconnect, ::accept의 함수들을 함수 포인터로 받아오기
	SOCKET dummySocket = CreateSocket();
	ASSERT_CRASH(BindWindowsFunction(dummySocket, WSAID_CONNECTEX, reinterpret_cast<LPVOID*>(&ConnectEx)));
	ASSERT_CRASH(BindWindowsFunction(dummySocket, WSAID_DISCONNECTEX, reinterpret_cast<LPVOID*>(&DisconnectEx)));
	ASSERT_CRASH(BindWindowsFunction(dummySocket, WSAID_ACCEPTEX, reinterpret_cast<LPVOID*>(&AcceptEx)));
	Close(dummySocket);
}

void SocketUtils::Clear() 
{
	::WSACleanup();
}

bool SocketUtils::BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* fn)
{
	//::connect, ::disconnect, ::accept를 직접 호출하지 않고
	//포인터로 받아서 사용
	
	//이유
	//https://stackoverflow.com/questions/4470645/acceptex-without-wsaioctl

	DWORD bytes = 0;
	return SOCKET_ERROR != ::WSAIoctl(
		socket,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guid, sizeof(guid),
		fn, sizeof(*fn),
		OUT & bytes, NULL, NULL);
}

SOCKET SocketUtils::CreateSocket()
{
	//윈도우에서 제공되는 소켓 만드는 법
	//::socket보다 좀 더 세분화해서 설정할 수 있음
	
	//마지막 인자를 넣어주면 WSA_FLAG_OVERLAPPED계열의 비동기 입출력을 사용할 수 있음
	return ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
}

bool SocketUtils::SetLinger(SOCKET socket, uint16 onoff, uint16 linger)
{
	//onoff = 0이면 closesockekt이 바로 리턴
	//아니면 linger초 만큼 대기(default는 0)

	LINGER option;
	option.l_onoff = onoff;
	option.l_linger = linger;
	return SetSocketOpt(socket, SOL_SOCKET, SO_LINGER, option);
}

bool SocketUtils::SetReuseAddress(SOCKET socket, bool flag)
{
	return SetSocketOpt(socket, SOL_SOCKET, SO_REUSEADDR, flag);
}

bool SocketUtils::SetRecvBufferSize(SOCKET socket, int32 size)
{
	return SetSocketOpt(socket, SOL_SOCKET, SO_RCVBUF, size);
}

bool SocketUtils::SetSendBufferSize(SOCKET socket, int32 size)
{
	return SetSocketOpt(socket, SOL_SOCKET, SO_SNDBUF, size);
}

bool SocketUtils::SetTcpNoDelay(SOCKET socket, bool flag)
{
	//네이글 알고리즘(뭉쳐보내기) 끄기 
	return SetSocketOpt(socket, SOL_SOCKET, TCP_NODELAY, flag);
}

bool SocketUtils::SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket) 
{
	return SetSocketOpt(socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, listenSocket);
}


bool SocketUtils::Bind(SOCKET socket, NetAddress netAddr)
{
	return SOCKET_ERROR != ::bind(socket, reinterpret_cast<const SOCKADDR*>(&netAddr.GetSocketAddr()), sizeof(SOCKADDR_IN));
}

bool SocketUtils::BindAnyAddress(SOCKET socket, uint16 port)
{
	SOCKADDR_IN myAddress;
	myAddress.sin_family = AF_INET;
	myAddress.sin_addr.s_addr = ::htonl(INADDR_ANY);
	myAddress.sin_port = ::htons(port);
	
	return (SOCKET_ERROR != ::bind(socket, reinterpret_cast<const SOCKADDR*>(&myAddress), sizeof(myAddress)));
}

bool SocketUtils::Listen(SOCKET socket, int32 backlog /*= SOMAXCONN*/)
{
	return SOCKET_ERROR != ::listen(socket, backlog);
}

void SocketUtils::Close(SOCKET& socket) 
{
	if (INVALID_SOCKET == socket)
		return;

	::closesocket(socket);
	socket = INVALID_SOCKET;
}




