#include "utils.h"

#include <memory>
#include <stdarg.h> 
#include <stdexcept>
#include <Windows.h>

namespace qqimpl
{
namespace utils
{
	std::wstring Utf8ToUnicode(std::string utf8_str)
	{
		if (utf8_str.empty())
			return std::wstring();

		const auto size_needed = MultiByteToWideChar(CP_UTF8, 0, &utf8_str.at(0), (int)utf8_str.size(), nullptr, 0);
		if (size_needed <= 0)
		{
			throw std::runtime_error("MultiByteToWideChar() failed: " + std::to_string(size_needed));
		}

		std::wstring result(size_needed, 0);
		MultiByteToWideChar(CP_UTF8, 0, &utf8_str.at(0), (int)utf8_str.size(), &result.at(0), size_needed);
		return result;
	}

	std::string UnicodeToUtf8(std::wstring utf16_str)
	{
		if (utf16_str.empty())
			return std::string();

		const auto size_needed = WideCharToMultiByte(CP_UTF8, 0, &utf16_str.at(0), (int)utf16_str.size(), nullptr, 0, nullptr, nullptr);
		if (size_needed <= 0)
		{
			throw std::runtime_error("WideCharToMultiByte() failed: " + std::to_string(size_needed));
		}

		std::string result(size_needed, 0);
		WideCharToMultiByte(CP_UTF8, 0, &utf16_str.at(0), (int)utf16_str.size(), &result.at(0), size_needed, nullptr, nullptr);
		return result;
	}

	int CheckPathInfo(std::string path)
	{
		struct _stat64i32 info;
		if (_stat(path.c_str(), &info) != 0) {  // does not exist
			return 0;
		}

		if (info.st_mode & S_IFDIR) // directory
			return 1;
		else
			return 2;
	}

	int CheckPathInfo(std::wstring path)
	{
		struct _stat64i32 info;
		if (_wstat(path.c_str(), &info) != 0) {  // does not exist
			return 0;
		}

		if (info.st_mode & S_IFDIR) // directory
			return 1;
		else
			return 2;
	}

	std::string string_format(const std::string fmt_str, ...)
	{
		int final_n, n = ((int)fmt_str.size()) * 2; /* Reserve two times as much as the length of the fmt_str */
		std::unique_ptr<char[]> formatted;
		va_list ap;
		while (1) {
			formatted.reset(new char[n]); /* Wrap the plain char array into the unique_ptr */
			strcpy_s(&formatted[0], n, fmt_str.c_str());
			va_start(ap, fmt_str);
			final_n = vsnprintf(&formatted[0], n, fmt_str.c_str(), ap);
			va_end(ap);
			if (final_n < 0 || final_n >= n)
				n += abs(final_n - n + 1);
			else
				break;
		}
		return std::string(formatted.get());
	}
}
}


