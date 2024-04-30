#include "pch.h"
#include "FileUtils.h"
#include <filesystem>
#include <fstream>

/*----------------------------
					FileUtils
----------------------------*/

Vector<BYTE> FileUtils::ReadFile(const WCHAR* path)
{
	Vector<BYTE> ret;

	std::filesystem::path filePath{ path };
	
	const uint32 fileSize = static_cast<uint32>(std::filesystem::file_size(filePath));
	ret.resize(fileSize);
	
	std::basic_ifstream<BYTE> inputStream{ filePath };
	inputStream.read(&ret[0], fileSize);

	return ret;
}

String FileUtils::Convert(std::string str)
{
	const int32 srcLen = static_cast<int32>(str.size());

	String ret;
	if (0 == srcLen)
		return ret;

	const int32 retLen = ::MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<char*>(&str[0]), srcLen, NULL, 0);
	ret.resize(retLen);
	::MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<char*>(&str[0]), srcLen, &ret[0], retLen);

	return ret;
}

void FileUtils::ReadFile(std::filesystem::path path, OUT Vector<BYTE>& datas)
{
	ASSERT_CRASH(std::filesystem::exists(path));
	datas.clear();
	
	FILE* filePtr = nullptr;
	std::wstring ioMode = L"rb";
	
	_wfopen_s(&filePtr, path.wstring().c_str(), ioMode.c_str());
	ASSERT_CRASH(filePtr);
	
	size_t size = std::filesystem::file_size(path);
	datas.resize(size);

	fread_s(&datas[0], datas.size(), size, 1, filePtr);
	fclose(filePtr);
}
