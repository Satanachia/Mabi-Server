#pragma once

class SendBufferChunk;

/*----------------------------------
					SendBuffer
----------------------------------*/

/*
		브로드캐스트 할 때 
		같은 데이터에 대한 복사를 줄이기 위해
		shared_ptr로 관리(경량패턴)

		참고로 아직까지는(SendBufferPool 적용 전)
		데이터를 모두 보내고 나면 이 SendBuffer는 소멸됨
*/
class SendBuffer
{
public:
	SendBuffer(SendBufferChunkRef owner, BYTE* buffer, uint32 allocSize);
	~SendBuffer();

	SendBuffer(const SendBuffer& _Other) = delete;
	SendBuffer(SendBuffer&& _Other) noexcept = delete;
	SendBuffer& operator=(const SendBuffer& _Other) = delete;
	SendBuffer& operator=(const SendBuffer&& _Other) noexcept = delete;

	//실제 사용중인 버퍼 첫 주소 반환
	BYTE* Buffer() { return _buffer; }

	uint32 AllocSize() { return _allocSize; }

	//이 버퍼에 적은 데이터 크기
	uint32 WriteSize() { return _writeSize; }
	
	//해당 버퍼 공간에 데이터 작성
	void Close(uint32 writeSize);

private:
	BYTE*							_buffer;
	uint32							_allocSize = 0;
	uint32							_writeSize = 0;
	SendBufferChunkRef _owner;
};

/*----------------------------------
					SendBufferChunk
----------------------------------*/


class SendBufferChunk : public std::enable_shared_from_this<SendBufferChunk>
{
	enum
	{
		//SEND_BUFFER_CHUNK_SIZE = 6000
		SEND_BUFFER_CHUNK_SIZE = 100000
	};
public:
	SendBufferChunk();
	~SendBufferChunk();

	void						Reset();

	/// <summary>
	/// SendBufferChunk 버퍼 안에서 사용할 지점을
	/// SendBufferRef로 래핑한 후 반환
	/// </summary>
	/// <param name="allocSize">SendBufferChunk버퍼에 allocSize만큼 공간이 있는지 확인</param>
	/// <returns>내부에서 사용할 지점을 가르키고 있음</returns>
	SendBufferRef		Open(uint32 allocSize);

	//해당 메모리를 다 사용한 뒤 반환(내부에서 _usedSize가 증가된다)
	void						Close(uint32 writeSize);

	bool		IsOpen()		{ return _open; }

	//이 ChunkBuffer에 작성할 시작 위치
	BYTE*	Buffer()		{ return &_buffer[_usedSize]; }

	//이 ChunkBuffer의 여유공간
	uint32	FreeSize()		{ return static_cast<uint32>(_buffer.size() - _usedSize); }

private:
	Array<BYTE, SEND_BUFFER_CHUNK_SIZE> _buffer = {};

	bool			_open			= false;
	uint32		_usedSize		= 0;
};


/*----------------------------------
					SendBufferManager
----------------------------------*/

class SendBufferManager
{
public:
	//SendBufferChunk에서 일부를 뜯어서 내주는 함수
	SendBufferRef Open(uint32 size);

private:
	//SendBufferChunk를 풀에서 꺼낸다
	SendBufferChunkRef Pop();

	//SendBufferChunk를 다 소진했을때 풀에 반납
	void Push(SendBufferChunkRef buffer);

	//SendBufferChunk의 refCount가 0이 되었을때 메모리를 날리지 않고 이 함수로 들어온다
	static void PushGlobal(SendBufferChunk* buffer);

private:
	USE_LOCK;
	Vector<SendBufferChunkRef> _sendBufferChunks;
};