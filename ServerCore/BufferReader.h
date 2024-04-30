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
	uint32 ReadSize()		{ return _pos; }	//������ �о�����
	uint32 FreeSize()		{ return (_size - _pos); };

	//�� ���ۿ� �ִ� ������ sizeof(T)��ŭ dest�� �̵�
	template <typename T>
	BufferReader& operator >>(OUT T& dest);

	//Ŀ��(_pos)�� �̵���Ű�� �ʰ� �����͸� �о���δ�
	bool Peek(void* dest, uint32 len);

	//Ŀ��(_pos)�� �̵���Ű�鼭 �����͸� �о���δ�
	bool Read(void* dest, uint32 len);
	
	//Ŀ��(_pos)�� �̵���Ű�� �ʰ� �����͸� �о���δ�
	template <typename T>
	bool Peek(T* dest) { return Peek(dest, sizeof(T)); }

	//Ŀ��(_pos)�� �̵���Ű�鼭 �����͸� �о���δ�
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
	//�ڽ��� ������ _pos���� dest �� �����ŭ ����
	dest = *reinterpret_cast<T*>(&_buffer[_pos]);
	_pos += sizeof(T);
	return *this;
}