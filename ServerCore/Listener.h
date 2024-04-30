#pragma once
#include "IocpCore.h"
#include "NetAddress.h"

/*----------------------------------
					Listener
----------------------------------*/

class AcceptEvent;
class ServerService;

class Listener : public IocpObject
{
public:
	Listener();
	~Listener();

	Listener(const Listener& _Other) = delete;
	Listener(Listener&& _Other) noexcept = delete;
	Listener& operator=(const Listener& _Other) = delete;
	Listener& operator=(const Listener&& _Other) noexcept = delete;

public:/*외부에서 사용*/
	//리스너 소켓의 클라 입장 실행
	bool StartAccept(ServerServiceRef service);

	void CloseSocket();


public:/*인터페이스*/
	virtual HANDLE GetHandle() override;
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) override;

private:/*수신 관련*/

	//클라가 서버에 접속하는지 감시 시작
	void RegisterAccept(AcceptEvent* acceptEvent);
	void ProcessAccept(AcceptEvent* acceptEvent);


protected:
	SOCKET _socket = INVALID_SOCKET;
	Vector<AcceptEvent*> _acceptEvents;

	//지금은 순환참조가 일어나고 있지만
	//나중엔 꼭 끊어줄 예정
	ServerServiceRef _service;
};

