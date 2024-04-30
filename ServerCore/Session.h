#pragma once
#include "IocpCore.h"
#include "IocpEvent.h"
#include "NetAddress.h"
#include "RecvBuffer.h"

/*----------------------------------
					Session
----------------------------------*/

class Service;

class Session : public IocpObject
{
	friend class Listener;
	friend class IocpCore;
	friend class Service;

	enum
	{
		BUFFER_SIZE = 0x10000 //64kb
	};

public:
	Session();
	virtual ~Session();

	Session(const Session& _Other) = delete;
	Session(Session&& _Other) noexcept = delete;
	Session& operator=(const Session& _Other) = delete;
	Session& operator=(const Session&& _Other) noexcept = delete;

public:/*외부에서 사용*/

	void										Send(SendBufferRef sendBuffer);
	bool										Connect();
	void										Disconnect(const WCHAR* cause);
	std::shared_ptr<Service>	GetService() { return _service.lock(); }
	void										SetService(std::shared_ptr<Service> service) { _service = service; }


public:/*세션 정보 관련*/

	void				SetNetAddress(NetAddress address) { _netAddress = address; }
	NetAddress	GetNetAddress()		{ return _netAddress; }
	SOCKET		GetSocket()				{ return _socket; }
	bool				IsConnected()			{ return _connected; }
	SessionRef	GetSessionRef()		{ return std::static_pointer_cast<Session>(shared_from_this()); }

	

private:/*인터페이스 구현*/

	//이 세션이 들고 있는 소켓을 핸들 형태로 반환함
	virtual HANDLE GetHandle() override;
	
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) override;
	
private:/*전송 관련*/

	/*
		Register계열 함수는 CP에 예약을 담당
	*/
	bool RegisterConnect();
	bool RegisterDisconnect();
	void RegisterRecv();
	void RegisterSend();

	/*
		Process계열 함수는 CP에서 빠져나온 이벤트 처리를 담당
	*/
	void ProcessConnect();
	void ProcessDisconnect();
	void ProcessRecv(int32 numOfBytes);
	void ProcessSend(int32 numOfBytes);/*SendEvent 인자 임시*/

	void HandleError(int32 errorCode);

protected:
	/*
		컨텐츠쪽에서 특정 시점에 호출되게 할
		오버라이딩 용 함수들
	*/
	virtual void OnConnected(){}

	//반환값 : 실제로 처리한 데이터 바이트 수
	virtual int32 OnRecv(BYTE* buffer, int32 len) { return len; }

	virtual void OnSend(int32 len){}

	virtual void OnDisconnected(){}

private:
	std::weak_ptr<Service>	_service;
	SOCKET							_socket			= INVALID_SOCKET;
	NetAddress						_netAddress	= {};
	Atomic<bool>					_connected		= false;

private:
	USE_LOCK;

	/*수신관련*/
	RecvBuffer						_recvBuffer;

	/*송신관련*/
	Queue<SendBufferRef>	_sendQueue;
	Atomic<bool>					_sendRegisterd = false;

private:/*IocpEvent 재활용*/

	ConnectEvent			_connectEvent;
	DisconnectEvent		_disconnectEvent;
	RecvEvent				_recvEvent;
	SendEvent				_sendEvent;
};

/*----------------------------------
					PacketSession
----------------------------------*/

struct PacketHeader
{
	uint16 size;
	uint16 id;
};

class PacketSession : public Session
{
public:
	PacketSession();
	virtual ~PacketSession();

	PacketSessionRef GetPacketSessionRef() { return std::static_pointer_cast<PacketSession>(shared_from_this()); }

protected:
	virtual int32		OnRecv			(BYTE* buffer, int32 len) sealed;
	virtual void		OnRecvPacket	(BYTE* buffer, int32 len) abstract;
};
