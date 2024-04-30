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
	//���� ���� �ִ��� Ȯ��
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

	//���� ���� ����
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
	//������ ������ �̿��ؼ� �ִ��� ���� ȸ��
	if (nullptr == LSendBufferChunk)
	{
		LSendBufferChunk = Pop();
		LSendBufferChunk->Reset();
	}

	//�̹� Open�ߴµ� �� Open�ϴ°� ����
	ASSERT_CRASH(false == LSendBufferChunk->IsOpen());

	//�� ���ۿ� ���̻� �ڸ��� ���ٸ� ������ ���ŷ� ��ü
	if (LSendBufferChunk->FreeSize() < size)
	{
		/*
			���� SendBufferChunk�� refCount�� ����
			refCount�� 0�� �ȴٸ� PushGlobal�� ȣ��
		*/
		LSendBufferChunk = Pop();
		LSendBufferChunk->Reset();
	}
	
	//���� ��¥ �غ�Ϸ�. ����� ���� ����ֱ�
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


	//(std::shared_ptr����. ���� MakeShared���鶧�� �Ȱ��� ����)
	//xnew�� shared_ptr<SendBufferChunk>�� �����ϰ� 
	//refCount�� 0�� �Ǹ� �޸𸮸� ������ �ʰ� PushGlobal�� ȣ��
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

	//�Ҹ��Ű�� �ʰ� ����(Ǯ�� ����)
	GSendBufferManager->Push(SendBufferChunkRef(buffer, PushGlobal));
}


