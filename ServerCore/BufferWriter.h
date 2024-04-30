#pragma once

/*----------------------------------
					BufferWriter
----------------------------------*/


class BufferWriter
{
public:
	BufferWriter();
	BufferWriter(BYTE* buffer, uint32 size, uint32 pos = 0);
	~BufferWriter();

	//BufferWriter(const BufferWriter& _Other) = delete;
	//BufferWriter(BufferWriter&& _Other) noexcept = delete;
	//BufferWriter& operator=(const BufferWriter& _Other) = delete;
	//BufferWriter& operator=(const BufferWriter&& _Other) noexcept = delete;

	BYTE* Buffer() { return _buffer; }
	uint32 Size() { return _size; }
	uint32 WriteSize() { return _pos; }	//어디까지 읽었는지
	uint32 FreeSize() { return (_size - _pos); };

	//템플릿 인자 크기만큼 공간을 예약한다(내부에서 _pos를 이미 움직여놓는다)
	template <typename T>
	T* Reserve(uint16 count = 1);


	template <typename T>
	BufferWriter& operator <<(T&& src);

	bool Write(void* src, uint32 len);

	template <typename T>
	bool Write(T* src) { return Read(src, sizeof(T)); }

private:
	BYTE* _buffer = nullptr;
	uint32 _size = 0;
	uint32 _pos = 0;
};

template<typename T>
inline T* BufferWriter::Reserve(uint16 count /*= 1*/)
{
	if (FreeSize() < (sizeof(T) * count))
		return nullptr;

	T* ret = reinterpret_cast<T*>(&_buffer[_pos]);
	//커서를 미리 이동시켜 놓는다
	_pos += (sizeof(T) * count);
	return ret;
}




template<typename T>
inline BufferWriter& BufferWriter::operator<<(T&& src)
{
	//T타입에서 &(레퍼런스)를 제외하겠다
	using DataType = std::remove_reference_t<T>;

	//src에 있는 데이터를 버퍼에 복사(memcpy와 똑같음)
	*reinterpret_cast<DataType*>(&_buffer[_pos]) = std::forward<DataType>(src);
	_pos += sizeof(T);
	return *this;
}


