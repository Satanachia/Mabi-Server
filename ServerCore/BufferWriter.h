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
	uint32 WriteSize() { return _pos; }	//������ �о�����
	uint32 FreeSize() { return (_size - _pos); };

	//���ø� ���� ũ�⸸ŭ ������ �����Ѵ�(���ο��� _pos�� �̹� ���������´�)
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
	//Ŀ���� �̸� �̵����� ���´�
	_pos += (sizeof(T) * count);
	return ret;
}




template<typename T>
inline BufferWriter& BufferWriter::operator<<(T&& src)
{
	//TŸ�Կ��� &(���۷���)�� �����ϰڴ�
	using DataType = std::remove_reference_t<T>;

	//src�� �ִ� �����͸� ���ۿ� ����(memcpy�� �Ȱ���)
	*reinterpret_cast<DataType*>(&_buffer[_pos]) = std::forward<DataType>(src);
	_pos += sizeof(T);
	return *this;
}


