#pragma once
#include <functional>

/*---------------------------------------------------
									Job
---------------------------------------------------*/

using CallbackType = std::function<void()>;

//JobQueue���� ó���� �ݹ��� ������ Ŭ����
class Job
{
public:
	//��� �Լ��� ������ �ݹ��� �޴� ������
	Job(CallbackType&& callback) : _callback(std::move(callback)){}

	/// <summary>
	/// ��� �Լ��� �ݹ��� �޴� ������
	/// </summary>
	/// <param name="owner">�ش� ��� �Լ��� ��ü</param>
	/// <param name="memFunc">��� �Լ�</param>
	/// <param name="...args">��� �Լ� ����</param>
	template <typename T, typename Ret, typename... Args>
	Job(std::shared_ptr<T> owner, Ret(T::* memFunc)(Args...), Args&&... args)
	{
		/*
			��� �Լ��� ��쿣 std::bind�� ��� �ϴµ�
			�׶� this�����͸� �ѱ�� �� �ƴ϶� shared_from_this�� �Ѱܾ� �Ѵ�.(��å��)
			�� �۾��� ���ŷο�� ������ ����Լ� �ݹ�� �Լ��� ���� ���ο��� ó��

			(�Ʒ� ���ٰ� std::bind�� ũ�� ������ ũ�� �ٸ��� ���� �� ����.
			bind�� �Ƹ� ������� ���ø����� ��ü, �Լ�������, ���ڸ� ���� ������ �ʹ�)
		*/
		_callback = [owner, memFunc, args...]()
		{
			(owner.get()->*memFunc)(args...);
		};
	}

	Job(const Job& _Other) = delete;
	Job(Job&& _Other) noexcept = delete;
	Job& operator=(const Job& _Other) = delete;
	Job& operator=(const Job&& _Other) noexcept = delete;

public:
	void Execute()
	{
		_callback();
	}

private:
	CallbackType _callback;
};

