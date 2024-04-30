#include "pch.h"
#include "Service.h"
#include "Session.h"
#include "Listener.h"

/*----------------------------------
					Service
----------------------------------*/

Service::Service(ServiceType type, NetAddress address, IocpCoreRef core, SessionFactory factory, int32 maxSessionCount)
	:_type(type)
	, _netAddress(address)
	,_iocpCore(core)
	,_sessionFactory(factory)
	,_maxSessionCount(maxSessionCount)
{
	
}

Service::~Service()
{

}

void Service::CloseService()
{
	//TODO
}

SessionRef Service::CreateSession()
{
	SessionRef session = _sessionFactory();
	session->SetService(shared_from_this());

	if (false == _iocpCore->Register(session))
		return nullptr;

	return session;
}

void Service::AddSession(SessionRef session)
{
	WRITE_LOCK;

	++_sessionCount;
	_sessions.insert(session);
}

void Service::ReleaseSession(SessionRef session)
{
	WRITE_LOCK;

	ASSERT_CRASH(0 != _sessions.erase(session));

	--_sessionCount;
}

void Service::Broadcast(SendBufferRef sendBuffer)
{
	WRITE_LOCK;

	for (const SessionRef& session : _sessions)
	{
		session->Send(sendBuffer);
	}
}


/*----------------------------------
					ClientService
----------------------------------*/

ClientService::ClientService(NetAddress targetAddress, IocpCoreRef core, SessionFactory factory, int32 maxSessionCount)
	:Service(ServiceType::Client, targetAddress, core, factory, maxSessionCount)
{

}

bool ClientService::Start()
{
	if (false == CanStart())
		return false;

	const int32 sessionCount = GetMaxSessionCount();
	for (int32 i = 0; i < sessionCount; ++i)
	{
		SessionRef session = CreateSession();
		if (false == session->Connect())
			return false;
	}

	//TODO
	return true;
}



/*----------------------------------
					ServerService
----------------------------------*/

ServerService::ServerService(NetAddress address, IocpCoreRef core, SessionFactory factory, int32 maxSessionCount)
	:Service(ServiceType::Server, address, core, factory, maxSessionCount)
{

}

bool ServerService::Start()
{
	if (false == CanStart())
		return false;

	//리스너 생성
	_listener = MakeShared<Listener>();
	if (nullptr == _listener)
		return false;

	//리스너 소켓을 CP에 등록 및
	//비동기 Accept 실행
	ServerServiceRef service = std::static_pointer_cast<ServerService>(shared_from_this());
	if (false == _listener->StartAccept(service))
		return false;

	return true;
}

void ServerService::CloseService()
{
	Service::CloseService();

	//TODO
}
