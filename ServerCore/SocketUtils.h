#pragma once
#include "NetAddress.h"

/*----------------------------------
					SocketUtils
----------------------------------*/

class SocketUtils
{
public:
	SocketUtils(){}
	~SocketUtils(){}

	SocketUtils(const SocketUtils& _Other) = delete;
	SocketUtils(SocketUtils&& _Other) noexcept = delete;
	SocketUtils& operator=(const SocketUtils& _Other) = delete;
	SocketUtils& operator=(const SocketUtils&& _Other) noexcept = delete;

	static LPFN_CONNECTEX			ConnectEx;
	static LPFN_DISCONNECTEX		DisconnectEx;
	static LPFN_ACCEPTEX				AcceptEx;

public:
	static void Init();

	//런타임에 위쪽 함수포인터와 연결시키는 코드
	static bool BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* fn);

	static void Clear();

	static SOCKET CreateSocket();



	//연결이 끊겼을 때 Send버퍼에 있던 데이터는 어떻게 처리할 지 설정
	static bool SetLinger(SOCKET socket, uint16 onoff, uint16 linger);
	//똑같은 주소와 포트를 재사용하겠다
	static bool SetReuseAddress(SOCKET socket, bool flag);
	//소켓의 수신 버퍼의 크기 설정
	static bool SetRecvBufferSize(SOCKET socket, int32 size);
	//소켓의 송신 버퍼의 크기 설정
	static bool SetSendBufferSize(SOCKET socket, int32 size);
	//커널단계에서 데이터 뭉쳐보내기 설정
	static bool SetTcpNoDelay(SOCKET socket, bool flag);
	//listen소켓의 특성을 클라이언트 소켓에 그대로 적용
	static bool SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket);



	//리슨 소켓과 서버 주소를 연동
	static bool Bind(SOCKET socket, NetAddress netAddr);
	//클라쪽에서 자신의 소켓과 서버주소를 연동
	static bool BindAnyAddress(SOCKET socket, uint16 port);


	static bool Listen(SOCKET socket, int32 backlog = SOMAXCONN);
	static void Close(SOCKET& socket);
};

//TCP_NODELAY, 버퍼크기, ip와 포트 재사용, 링거에 대한 옵션을 설정할 때 사용되는 템플릿 함수
template <typename T>
static inline bool SetSocketOpt(SOCKET socket, int32 level, int32 optName, T optVal)
{
	return SOCKET_ERROR != ::setsockopt(socket, level, optName, reinterpret_cast<char*>(&optVal), sizeof(T));
}
