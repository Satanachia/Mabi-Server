#include "pch.h"
#include "SendBuffer.h"

SendBuffer::SendBuffer(SendBufferChunkRef owner, BYTE* buffer, uint32 allocSize)
	:_owner(owner), _buffer(buffer), _allocSize(allocSize)
{

}

SendBuffer::~SendBuffer()
{
	
}

void SendBuffer::Close(uint32 writeSize)
{
	//여유 공간 있는지 확인
	ASSERT_CRASH(writeSize <= _allocSize);
	_writeSize = writeSize;
	_owner->Close(writeSize);
}


/*----------------------------------
					SendBufferChunk
----------------------------------*/

SendBufferChunk::SendBufferChunk()
{
}

SendBufferChunk::~SendBufferChunk()
{
}

void SendBufferChunk::Reset()
{
	_open = false;
	_usedSize = 0;
}

SendBufferRef SendBufferChunk::Open(uint32 allocSize)
{
	ASSERT_CRASH(allocSize <= SEND_BUFFER_CHUNK_SIZE);
	ASSERT_CRASH(false == _open);

	//여유 공간 부족
	if (FreeSize() < allocSize)
		return nullptr;

	_open = true;
	return ObjectPool<SendBuffer>::MakeShared(shared_from_this(), Buffer(), allocSize);
}

void SendBufferChunk::Close(uint32 writeSize)
{
	ASSERT_CRASH(true == _open);

	_open = false;
	_usedSize += writeSize;
}

/*----------------------------------
					SendBufferManager
----------------------------------*/

SendBufferRef SendBufferManager::Open(uint32  size)
{
	//스레드 로컬을 이용해서 최대한 락을 회피
	if (nullptr == LSendBufferChunk)
	{
		LSendBufferChunk = Pop();
		LSendBufferChunk->Reset();
	}

	//이미 Open했는데 또 Open하는것 방지
	ASSERT_CRASH(false == LSendBufferChunk->IsOpen());

	//이 버퍼에 더이상 자리가 없다면 버리고 새거로 교체
	if (LSendBufferChunk->FreeSize() < size)
	{
		/*
			기존 SendBufferChunk는 refCount가 감소
			refCount가 0이 된다면 PushGlobal이 호출
		*/
		LSendBufferChunk = Pop();
		LSendBufferChunk->Reset();
	}
	
	//이제 진짜 준비완료. 사용할 영역 뜯어주기
	return LSendBufferChunk->Open(size);
}

SendBufferChunkRef SendBufferManager::Pop()
{
	{
		WRITE_LOCK;

		if (false == _sendBufferChunks.empty())
		{
			SendBufferChunkRef sendBufferChunk = _sendBufferChunks.back();
			_sendBufferChunks.pop_back();
			return sendBufferChunk;
		}
	}


	//(std::shared_ptr문법. 전에 MakeShared만들때도 똑같이 썼음)
	//xnew로 shared_ptr<SendBufferChunk>을 생성하고 
	//refCount가 0이 되면 메모리를 날리지 않고 PushGlobal를 호출
	return SendBufferChunkRef(xnew<SendBufferChunk>(), PushGlobal);
}

void SendBufferManager::Push(SendBufferChunkRef buffer)
{
	WRITE_LOCK;

	_sendBufferChunks.push_back(buffer);
}

void SendBufferManager::PushGlobal(SendBufferChunk* buffer)
{
	std::cout << "PushGlobal SENDBUFFERCHUNK" << std::endl;

	//소멸시키지 않고 재사용(풀에 저장)
	GSendBufferManager->Push(SendBufferChunkRef(buffer, PushGlobal));
}


