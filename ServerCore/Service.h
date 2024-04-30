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
	하나의 네트워크 주소에 대해서
	Iocp와 세션들을 관리하는 클래스
	(서버임에도 네트워크 주소와 Iocp, 리스너까지 관리하는 이유는, 현업에서 분산서버를 이용하기도 하기 때문)
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

	//세션을 만들면서 iocp코어에 등록
	SessionRef CreateSession();
	//세션 추가
	void AddSession(SessionRef session);
	//세션 사용 종료
	void ReleaseSession(SessionRef session);
	//현재 동접자 수
	int32 GetCurrentSessionCount() { return _sessionCount; }
	//최대 동접자 수
	int32 GetMaxSessionCount() { return _maxSessionCount; }

	ServiceType GetServiceType() { return _type; }
	NetAddress GetNetAddress() { return _netAddress; }
	IocpCoreRef& GetIocpCore() { return _iocpCore; }

	//클라쪽에서 채팅 연습을 하기 위한 임시함수
	void Broadcast(SendBufferRef sendBuffer);

protected:
	USE_LOCK;

	ServiceType			_type;
	NetAddress			_netAddress = {};
	IocpCoreRef		_iocpCore;
	
	Set<SessionRef>	_sessions;
	int32					_sessionCount = 0;
	int32					_maxSessionCount = 0;
	
	//컨텐츠 쪽에서 어떤 세션을 만들어 줄지 결정할 수 있게
	//세션을 만드는 부분을 콜백으로 받음
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

	//리스너를 만들고, 비동기 Accept를 시작
	virtual bool Start() override;
	virtual void CloseService() override;

private:
	ListenerRef _listener = nullptr;
};