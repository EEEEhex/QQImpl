#pragma once

#include <string>

namespace qqimpl
{
namespace utils
{
	//一些工具函数
	
	//以下为char与wchar的转换
	std::wstring Utf8ToUnicode(std::string utf8_str);
	std::string UnicodeToUtf8(std::wstring utf16_str);

	//返回0->不存在 1->是文件夹 2->是文件
	int CheckPathInfo(std::string path);
	int CheckPathInfo(std::wstring path);

	std::string string_format(const std::string fmt_str, ...);
}
}
