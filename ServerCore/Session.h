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

public:/*�ܺο��� ���*/

	void										Send(SendBufferRef sendBuffer);
	bool										Connect();
	void										Disconnect(const WCHAR* cause);
	std::shared_ptr<Service>	GetService() { return _service.lock(); }
	void										SetService(std::shared_ptr<Service> service) { _service = service; }


public:/*���� ���� ����*/

	void				SetNetAddress(NetAddress address) { _netAddress = address; }
	NetAddress	GetNetAddress()		{ return _netAddress; }
	SOCKET		GetSocket()				{ return _socket; }
	bool				IsConnected()			{ return _connected; }
	SessionRef	GetSessionRef()		{ return std::static_pointer_cast<Session>(shared_from_this()); }

	

private:/*�������̽� ����*/

	//�� ������ ��� �ִ� ������ �ڵ� ���·� ��ȯ��
	virtual HANDLE GetHandle() override;
	
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) override;
	
private:/*���� ����*/

	/*
		Register�迭 �Լ��� CP�� ������ ���
	*/
	bool RegisterConnect();
	bool RegisterDisconnect();
	void RegisterRecv();
	void RegisterSend();

	/*
		Process�迭 �Լ��� CP���� �������� �̺�Ʈ ó���� ���
	*/
	void ProcessConnect();
	void ProcessDisconnect();
	void ProcessRecv(int32 numOfBytes);
	void ProcessSend(int32 numOfBytes);/*SendEvent ���� �ӽ�*/

	void HandleError(int32 errorCode);

protected:
	/*
		�������ʿ��� Ư�� ������ ȣ��ǰ� ��
		�������̵� �� �Լ���
	*/
	virtual void OnConnected(){}

	//��ȯ�� : ������ ó���� ������ ����Ʈ ��
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

	/*���Ű���*/
	RecvBuffer						_recvBuffer;

	/*�۽Ű���*/
	Queue<SendBufferRef>	_sendQueue;
	Atomic<bool>					_sendRegisterd = false;

private:/*IocpEvent ��Ȱ��*/

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
