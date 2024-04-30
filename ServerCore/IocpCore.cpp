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
		iocpObject->GetHandle(),	//소켓
		_iocpHandle, 
		/*reinterpret_cast<ULONG_PTR>(iocpObject)*/0, 0);
	
	//이젠 iocpObject를 키값으로 넘기지 않고
	//비동기 입출력을 호출할때 OVERLAPPED쪽에서
	//shared_ptr을 소유한 채로 넘긴다

	//그냥 넘기면 iocpObject가 다른곳에서 삭제되서
	//댕글링 포인터가 될 수 있기 때문에
	//shared_ptr로 관리

	return result;
}

bool IocpCore::Dispatch(uint32 timeoutMs /*= INFINITE*/)
{
	DWORD numOfBytes = 0;
	ULONG_PTR key = 0;	//사용 안함
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
			//진짜 문제 있음. 근데 일단은 Dispatch해주자
			IocpObjectRef iocpObject = iocpEvent->owner;
			iocpObject->Dispatch(iocpEvent, numOfBytes);
			break;
		}
		}
	}
	
	return true;
}
