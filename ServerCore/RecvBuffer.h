#pragma once

/*-------------------------------------
								RecvBuffer
-------------------------------------*/
class RecvBuffer
{
	enum {BUFFER_COUNT = 10};

public:
	RecvBuffer(int32 bufferSize);
	~RecvBuffer();

	RecvBuffer(const RecvBuffer& _Other) = delete;
	RecvBuffer(RecvBuffer&& _Other) noexcept = delete;
	RecvBuffer& operator=(const RecvBuffer& _Other) = delete;
	RecvBuffer& operator=(const RecvBuffer&& _Other) noexcept = delete;

	//������ Ŀ�� ���� ��Ȳ�� ���� �̵�
	void Clean();
	//readĿ���� �̵����� ������ �����͸� �д´�
	bool OnRead(int32 numOfBytes);
	//writeĿ���� �̵����� �����͸� ���ۿ� ����. (���� ���ۿ� �ۼ��� Iocp�� �����Ű� writeĿ���� �̵�)
	bool OnWrite(int32 numOfBytes);

	//���� ���ۿ��� �о�� �ϴ� ����
	BYTE*	ReadPos()	{ return &_buffer[_readPos]; }

	//���� ���ۿ� ����ϴ� ����
	BYTE*	WritePos()	{ return &_buffer[_writePos]; }

	//��ȿ�� ������ ������
	int32	DataSize()	{ return (_writePos - _readPos); }

	//�� �ۼ��� �� �ִ� ���� ������
	int32	FreeSize()		{ return (_capacity - _writePos); }

private:
	int32				_capacity		= 0;
	int32				_bufferSize	= 0;
	int32				_readPos		= 0;
	int32				_writePos	= 0;
	Vector<BYTE>	_buffer;
};

