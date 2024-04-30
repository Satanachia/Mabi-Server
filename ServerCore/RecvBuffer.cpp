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

	//readĿ���� writeĿ���� �����Ƿ� 0���� �̵�
	if (0 == dataSize)
	{
		_readPos = 0;
		_writePos = 0;
		return;
	}

	//���� ������ ���� 1�� ũ�� �̸��̸� �����͸� ������ �̵�
	if (FreeSize() < _bufferSize)
	{
		//��ȿ�� �����͸� ���� �������� �̵�
		::memcpy(&_buffer[0], &_buffer[_readPos], dataSize);
		_readPos = 0;
		_writePos = dataSize;
	}
}

bool RecvBuffer::OnRead(int32 numOfBytes)
{
	//���ۿ� ����� �����ͺ��� �� ���� �������� �õ��Ѵ�
	if (DataSize() < numOfBytes)
		return false;

	_readPos += numOfBytes;
	return true;
}

bool RecvBuffer::OnWrite(int32 numOfBytes)
{
	//���� �ִ� �������� �� ���� �ۼ��Ϸ��� �Ѵ�
	if (FreeSize() < numOfBytes)
		return false;

	_writePos += numOfBytes;
	return true;
}
