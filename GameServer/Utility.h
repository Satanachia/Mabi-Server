#pragma once
#include <random>

class Utility
{
public:
	template <typename T>
	static T GetRandom(T min, T max)
	{
		std::random_device randomDevice;
		std::mt19937 generator(randomDevice());

		//constexpr는 컴파일 타임에 분기처리를 하여 템플릿 함수를 만들어 낸다
		
		//T가 int인 경우
		if constexpr (std::is_integral_v<T>)
		{
			std::uniform_int_distribution<T> distribution(min, max);
			return distribution(generator);
		}

		//T가 float인 경우
		else
		{
			std::uniform_real_distribution<T> distribution(min, max);
			return distribution(generator);
		}
	}

	static std::wstring AnsiToUniCode(const std::string_view& _Text);
	static std::string UniCodeToAnsi(const std::wstring_view& _Text);


};

