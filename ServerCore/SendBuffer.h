#pragma once

class SendBufferChunk;

/*----------------------------------
					SendBuffer
----------------------------------*/

/*
		��ε�ĳ��Ʈ �� �� 
		���� �����Ϳ� ���� ���縦 ���̱� ����
		shared_ptr�� ����(�淮����)

		����� ����������(SendBufferPool ���� ��)
		�����͸� ��� ������ ���� �� SendBuffer�� �Ҹ��
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

	//���� ������� ���� ù �ּ� ��ȯ
	BYTE* Buffer() { return _buffer; }

	uint32 AllocSize() { return _allocSize; }

	//�� ���ۿ� ���� ������ ũ��
	uint32 WriteSize() { return _writeSize; }
	
	//�ش� ���� ������ ������ �ۼ�
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
	/// SendBufferChunk ���� �ȿ��� ����� ������
	/// SendBufferRef�� ������ �� ��ȯ
	/// </summary>
	/// <param name="allocSize">SendBufferChunk���ۿ� allocSize��ŭ ������ �ִ��� Ȯ��</param>
	/// <returns>���ο��� ����� ������ ����Ű�� ����</returns>
	SendBufferRef		Open(uint32 allocSize);

	//�ش� �޸𸮸� �� ����� �� ��ȯ(���ο��� _usedSize�� �����ȴ�)
	void						Close(uint32 writeSize);

	bool		IsOpen()		{ return _open; }

	//�� ChunkBuffer�� �ۼ��� ���� ��ġ
	BYTE*	Buffer()		{ return &_buffer[_usedSize]; }

	//�� ChunkBuffer�� ��������
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
	//SendBufferChunk���� �Ϻθ� �� ���ִ� �Լ�
	SendBufferRef Open(uint32 size);

private:
	//SendBufferChunk�� Ǯ���� ������
	SendBufferChunkRef Pop();

	//SendBufferChunk�� �� ���������� Ǯ�� �ݳ�
	void Push(SendBufferChunkRef buffer);

	//SendBufferChunk�� refCount�� 0�� �Ǿ����� �޸𸮸� ������ �ʰ� �� �Լ��� ���´�
	static void PushGlobal(SendBufferChunk* buffer);

private:
	USE_LOCK;
	Vector<SendBufferChunkRef> _sendBufferChunks;
};