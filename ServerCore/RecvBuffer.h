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

	//데이터 커서 보고 상황에 따라 이동
	void Clean();
	//read커서를 이동시켜 버퍼의 데이터를 읽는다
	bool OnRead(int32 numOfBytes);
	//write커서를 이동시켜 데이터를 버퍼에 쓴다. (실제 버퍼에 작성은 Iocp가 했을거고 write커서만 이동)
	bool OnWrite(int32 numOfBytes);

	//현재 버퍼에서 읽어야 하는 지점
	BYTE*	ReadPos()	{ return &_buffer[_readPos]; }

	//현재 버퍼에 써야하는 지점
	BYTE*	WritePos()	{ return &_buffer[_writePos]; }

	//유효한 데이터 사이즈
	int32	DataSize()	{ return (_writePos - _readPos); }

	//더 작성할 수 있는 버퍼 사이즈
	int32	FreeSize()		{ return (_capacity - _writePos); }

private:
	int32				_capacity		= 0;
	int32				_bufferSize	= 0;
	int32				_readPos		= 0;
	int32				_writePos	= 0;
	Vector<BYTE>	_buffer;
};

