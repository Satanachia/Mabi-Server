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

	//��Ÿ�ӿ� ���� �Լ������Ϳ� �����Ű�� �ڵ�
	static bool BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* fn);

	static void Clear();

	static SOCKET CreateSocket();



	//������ ������ �� Send���ۿ� �ִ� �����ʹ� ��� ó���� �� ����
	static bool SetLinger(SOCKET socket, uint16 onoff, uint16 linger);
	//�Ȱ��� �ּҿ� ��Ʈ�� �����ϰڴ�
	static bool SetReuseAddress(SOCKET socket, bool flag);
	//������ ���� ������ ũ�� ����
	static bool SetRecvBufferSize(SOCKET socket, int32 size);
	//������ �۽� ������ ũ�� ����
	static bool SetSendBufferSize(SOCKET socket, int32 size);
	//Ŀ�δܰ迡�� ������ ���ĺ����� ����
	static bool SetTcpNoDelay(SOCKET socket, bool flag);
	//listen������ Ư���� Ŭ���̾�Ʈ ���Ͽ� �״�� ����
	static bool SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket);



	//���� ���ϰ� ���� �ּҸ� ����
	static bool Bind(SOCKET socket, NetAddress netAddr);
	//Ŭ���ʿ��� �ڽ��� ���ϰ� �����ּҸ� ����
	static bool BindAnyAddress(SOCKET socket, uint16 port);


	static bool Listen(SOCKET socket, int32 backlog = SOMAXCONN);
	static void Close(SOCKET& socket);
};

//TCP_NODELAY, ����ũ��, ip�� ��Ʈ ����, ���ſ� ���� �ɼ��� ������ �� ���Ǵ� ���ø� �Լ�
template <typename T>
static inline bool SetSocketOpt(SOCKET socket, int32 level, int32 optName, T optVal)
{
	return SOCKET_ERROR != ::setsockopt(socket, level, optName, reinterpret_cast<char*>(&optVal), sizeof(T));
}
