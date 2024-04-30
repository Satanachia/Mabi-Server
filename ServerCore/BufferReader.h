#pragma once

/*----------------------------------
					BufferReader
----------------------------------*/

class BufferReader
{
public:
	BufferReader();
	BufferReader(BYTE* buffer, uint32 size, uint32 pos = 0);
	~BufferReader();

	BufferReader(const BufferReader& _Other) = delete;
	BufferReader(BufferReader&& _Other) noexcept = delete;
	BufferReader& operator=(const BufferReader& _Other) = delete;
	BufferReader& operator=(const BufferReader&& _Other) noexcept = delete;
	
	BYTE* Buffer()		{ return _buffer; }
	uint32 Size()				{ return _size; }
	uint32 ReadSize()		{ return _pos; }	//어디까지 읽었는지
	uint32 FreeSize()		{ return (_size - _pos); };

	//이 버퍼에 있는 내용을 sizeof(T)만큼 dest에 이동
	template <typename T>
	BufferReader& operator >>(OUT T& dest);

	//커서(_pos)는 이동시키지 않고 데이터를 읽어들인다
	bool Peek(void* dest, uint32 len);

	//커서(_pos)를 이동시키면서 데이터를 읽어들인다
	bool Read(void* dest, uint32 len);
	
	//커서(_pos)는 이동시키지 않고 데이터를 읽어들인다
	template <typename T>
	bool Peek(T* dest) { return Peek(dest, sizeof(T)); }

	//커서(_pos)를 이동시키면서 데이터를 읽어들인다
	template <typename T>
	bool Read(T* dest) { return Read(dest, sizeof(T)); }

private:
	BYTE* _buffer = nullptr;
	uint32 _size = 0;
	uint32 _pos = 0;
};

template <typename T>
BufferReader& BufferReader::operator >>(OUT T& dest)
{
	//자신의 버퍼의 _pos부터 dest 의 사이즈만큼 복사
	dest = *reinterpret_cast<T*>(&_buffer[_pos]);
	_pos += sizeof(T);
	return *this;
}