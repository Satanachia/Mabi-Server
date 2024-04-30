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

		//constexpr�� ������ Ÿ�ӿ� �б�ó���� �Ͽ� ���ø� �Լ��� ����� ����
		
		//T�� int�� ���
		if constexpr (std::is_integral_v<T>)
		{
			std::uniform_int_distribution<T> distribution(min, max);
			return distribution(generator);
		}

		//T�� float�� ���
		else
		{
			std::uniform_real_distribution<T> distribution(min, max);
			return distribution(generator);
		}
	}

	static std::wstring AnsiToUniCode(const std::string_view& _Text);
	static std::string UniCodeToAnsi(const std::wstring_view& _Text);


};

