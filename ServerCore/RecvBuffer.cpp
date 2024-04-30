#include "pch.h"
#include "RecvBuffer.h"

RecvBuffer::RecvBuffer(int32 bufferSize)
	:_bufferSize(bufferSize)
{
	_capacity = (_bufferSize * BUFFER_COUNT);
	_buffer.resize(_capacity);
}

RecvBuffer::~RecvBuffer()
{

}

void RecvBuffer::Clean()
{
	int32 dataSize = DataSize();

	//read커서와 write커서가 같으므로 0으로 이동
	if (0 == dataSize)
	{
		_readPos = 0;
		_writePos = 0;
		return;
	}

	//여유 공간이 버퍼 1개 크기 미만이면 데이터를 앞으로 이동
	if (FreeSize() < _bufferSize)
	{
		//유효한 데이터를 버퍼 앞쪽으로 이동
		::memcpy(&_buffer[0], &_buffer[_readPos], dataSize);
		_readPos = 0;
		_writePos = dataSize;
	}
}

bool RecvBuffer::OnRead(int32 numOfBytes)
{
	//버퍼에 저장된 데이터보다 더 많이 읽을려도 시도한다
	if (DataSize() < numOfBytes)
		return false;

	_readPos += numOfBytes;
	return true;
}

bool RecvBuffer::OnWrite(int32 numOfBytes)
{
	//남아 있는 공간보다 더 많이 작성하려고 한다
	if (FreeSize() < numOfBytes)
		return false;

	_writePos += numOfBytes;
	return true;
}
