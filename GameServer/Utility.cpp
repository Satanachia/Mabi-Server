#include "pch.h"
#include "Utility.h"

std::wstring Utility::AnsiToUniCode(const std::string_view& _Text)
{
	int size = MultiByteToWideChar(CP_ACP, 0, _Text.data(), static_cast<int>(_Text.size()), nullptr, 0);
	if (0 == size)
		return L"";

	std::wstring result;
	result.resize(size);
	
	size = MultiByteToWideChar(CP_ACP, 0, _Text.data(), static_cast<int>(_Text.size()), &result[0], size);
	if (0 == size)
		return L"";
	
	return result;
}

std::string Utility::UniCodeToAnsi(const std::wstring_view& _Text)
{
	int size = WideCharToMultiByte(CP_ACP, 0, _Text.data(), static_cast<int>(_Text.size()), nullptr, 0, nullptr, nullptr);

	if (0 == size)
		return "";
	
	std::string result;
	result.resize(size);
	
	size = WideCharToMultiByte(CP_ACP, 0, _Text.data(), static_cast<int>(_Text.size()), &result[0], size, nullptr, nullptr);

	if (0 == size)
		return "";

	return result;
}
