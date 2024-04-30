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

public:/*�ܺο��� ���*/
	//������ ������ Ŭ�� ���� ����
	bool StartAccept(ServerServiceRef service);

	void CloseSocket();


public:/*�������̽�*/
	virtual HANDLE GetHandle() override;
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) override;

private:/*���� ����*/

	//Ŭ�� ������ �����ϴ��� ���� ����
	void RegisterAccept(AcceptEvent* acceptEvent);
	void ProcessAccept(AcceptEvent* acceptEvent);


protected:
	SOCKET _socket = INVALID_SOCKET;
	Vector<AcceptEvent*> _acceptEvents;

	//������ ��ȯ������ �Ͼ�� ������
	//���߿� �� ������ ����
	ServerServiceRef _service;
};

