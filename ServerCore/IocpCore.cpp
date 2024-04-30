#include "pch.h"
#include "IocpCore.h"
#include "IocpEvent.h"


IocpCore::IocpCore()
{
	_iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	ASSERT_CRASH(INVALID_HANDLE_VALUE != _iocpHandle);
}

IocpCore::~IocpCore()
{
	::CloseHandle(_iocpHandle);
}

bool IocpCore::Register(IocpObjectRef iocpObject)
{
	HANDLE result = ::CreateIoCompletionPort(
		iocpObject->GetHandle(),	//����
		_iocpHandle, 
		/*reinterpret_cast<ULONG_PTR>(iocpObject)*/0, 0);
	
	//���� iocpObject�� Ű������ �ѱ��� �ʰ�
	//�񵿱� ������� ȣ���Ҷ� OVERLAPPED�ʿ���
	//shared_ptr�� ������ ä�� �ѱ��

	//�׳� �ѱ�� iocpObject�� �ٸ������� �����Ǽ�
	//��۸� �����Ͱ� �� �� �ֱ� ������
	//shared_ptr�� ����

	return result;
}

bool IocpCore::Dispatch(uint32 timeoutMs /*= INFINITE*/)
{
	DWORD numOfBytes = 0;
	ULONG_PTR key = 0;	//��� ����
	IocpEvent* iocpEvent = nullptr;

	if (TRUE == ::GetQueuedCompletionStatus(
		_iocpHandle,
		OUT & numOfBytes,
		OUT &key,
		OUT reinterpret_cast<LPOVERLAPPED*>(&iocpEvent),
		timeoutMs))
	{
		IocpObjectRef iocpObject = iocpEvent->owner;
		iocpObject->Dispatch(iocpEvent, numOfBytes);
	}
	else
	{
		int32 errCore = ::WSAGetLastError();
		switch (errCore)
		{
		case WAIT_TIMEOUT:
			return false;
		default:
		{
			//��¥ ���� ����. �ٵ� �ϴ��� Dispatch������
			IocpObjectRef iocpObject = iocpEvent->owner;
			iocpObject->Dispatch(iocpEvent, numOfBytes);
			break;
		}
		}
	}
	
	return true;
}
