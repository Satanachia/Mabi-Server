#pragma once
#include <filesystem>

/*----------------------------
					FileUtils
----------------------------*/


class FileUtils
{
public:
	//���� �о���̱�
	static Vector<BYTE>	ReadFile(const WCHAR* path);
	//MBCS -> WBCS
	static String				Convert(std::string str);

	static void ReadFile(std::filesystem::path path, OUT Vector<BYTE>& datas);
};

