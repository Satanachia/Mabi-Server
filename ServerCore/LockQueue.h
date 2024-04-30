#pragma once

template <typename T>
class LockQueue
{
public:
	LockQueue(){}
	~LockQueue(){}

	LockQueue(const LockQueue& _Other) = delete;
	LockQueue(LockQueue&& _Other) noexcept = delete;
	LockQueue& operator=(const LockQueue& _Other) = delete;
	LockQueue& operator=(const LockQueue&& _Other) noexcept = delete;

public:
	void Push(T item)
	{
		WRITE_LOCK;
		_items.push(item);
	}

	T Pop()
	{
		WRITE_LOCK;

		if (true == _items.empty())
			return T();

		T ret = _items.front();
		_items.pop();
		return ret;
	}

	void PopAll(OUT Vector<T>& items)
	{
		WRITE_LOCK;

		items.reserve(items.size());
		while (T item = Pop())
			items.push_back(item);
	}

	void Clear()
	{
		WRITE_LOCK;
		_items = Queue<T>();
	}

private:
	USE_LOCK;

	Queue<T> _items;
};