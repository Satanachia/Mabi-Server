#pragma once
#include <functional>

#include "NetAddress.h"
#include "IocpCore.h"
#include "Listener.h"


enum class ServiceType : uint8
{
	Server,
	Client
};

/*----------------------------------
					Service
----------------------------------*/

/*
	�ϳ��� ��Ʈ��ũ �ּҿ� ���ؼ�
	Iocp�� ���ǵ��� �����ϴ� Ŭ����
	(�����ӿ��� ��Ʈ��ũ �ּҿ� Iocp, �����ʱ��� �����ϴ� ������, �������� �л꼭���� �̿��ϱ⵵ �ϱ� ����)
*/

using SessionFactory = std::function<SessionRef(void)>;

class Service : public std::enable_shared_from_this<Service>
{
public:
	Service(
		ServiceType type, 
		NetAddress address, 
		IocpCoreRef core, 
		SessionFactory factory, 
		int32 maxSessionCount = 1);
	virtual ~Service();

	Service(const Service& _Other) = delete;
	Service(Service&& _Other) noexcept = delete;
	Service& operator=(const Service& _Other) = delete;
	Service& operator=(const Service&& _Other) noexcept = delete;

public:
	virtual bool Start() abstract;
	bool CanStart() { return (nullptr != _sessionFactory); }
	virtual void CloseService();
	void SetSessionFactory(SessionFactory func) { _sessionFactory = func; }

	//������ ����鼭 iocp�ھ ���
	SessionRef CreateSession();
	//���� �߰�
	void AddSession(SessionRef session);
	//���� ��� ����
	void ReleaseSession(SessionRef session);
	//���� ������ ��
	int32 GetCurrentSessionCount() { return _sessionCount; }
	//�ִ� ������ ��
	int32 GetMaxSessionCount() { return _maxSessionCount; }

	ServiceType GetServiceType() { return _type; }
	NetAddress GetNetAddress() { return _netAddress; }
	IocpCoreRef& GetIocpCore() { return _iocpCore; }

	//Ŭ���ʿ��� ä�� ������ �ϱ� ���� �ӽ��Լ�
	void Broadcast(SendBufferRef sendBuffer);

protected:
	USE_LOCK;

	ServiceType			_type;
	NetAddress			_netAddress = {};
	IocpCoreRef		_iocpCore;
	
	Set<SessionRef>	_sessions;
	int32					_sessionCount = 0;
	int32					_maxSessionCount = 0;
	
	//������ �ʿ��� � ������ ����� ���� ������ �� �ְ�
	//������ ����� �κ��� �ݹ����� ����
	//ex) GameSession, ClientSession, Session ...
	SessionFactory	_sessionFactory;
};

/*----------------------------------
					ClientService
----------------------------------*/

class ClientService : public Service
{
public:
	ClientService(NetAddress targetAddress, IocpCoreRef core, SessionFactory factory, int32 maxSessionCount = 1);
	virtual ~ClientService() {}

	virtual bool Start() override;
};



/*----------------------------------
					ServerService
----------------------------------*/

class ServerService : public Service
{
public:
	ServerService(NetAddress address, IocpCoreRef core, SessionFactory factory, int32 maxSessionCount = 1);
	virtual ~ServerService() {}

	//�����ʸ� �����, �񵿱� Accept�� ����
	virtual bool Start() override;
	virtual void CloseService() override;

private:
	ListenerRef _listener = nullptr;
};