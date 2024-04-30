#pragma once
#include <functional>

/*---------------------------------------------------
									Job
---------------------------------------------------*/

using CallbackType = std::function<void()>;

//JobQueue에서 처리될 콜백을 래핑한 클래스
class Job
{
public:
	//멤버 함수를 제외한 콜백을 받는 생성자
	Job(CallbackType&& callback) : _callback(std::move(callback)){}

	/// <summary>
	/// 멤버 함수용 콜백을 받는 생성자
	/// </summary>
	/// <param name="owner">해당 멤버 함수의 객체</param>
	/// <param name="memFunc">멤버 함수</param>
	/// <param name="...args">멤버 함수 인자</param>
	template <typename T, typename Ret, typename... Args>
	Job(std::shared_ptr<T> owner, Ret(T::* memFunc)(Args...), Args&&... args)
	{
		/*
			멤버 함수의 경우엔 std::bind를 써야 하는데
			그때 this포인터를 넘기는 게 아니라 shared_from_this를 넘겨야 한다.(정책상)
			이 작업이 번거로우니 별도의 멤버함수 콜백용 함수를 만들어서 내부에서 처리

			(아래 람다가 std::bind와 크게 원리는 크게 다르지 않을 것 같다.
			bind는 아마 베리어딕 템플릿으로 객체, 함수포인터, 인자를 받지 않을까 싶다)
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

