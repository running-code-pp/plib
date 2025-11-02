#ifndef _core_utils_string_util_hpp_
#define _core_utils_string_util_hpp_
#include <string_view>
#include <string>
namespace plib::core::utils
{
	const char *const whitespaceDelimiters = " \t\n\r\f\v";
#ifdef _WIN32
	const char *const endl = "\r\n";
#else
	const char *const endl = "\n";
#endif
	[[nodiscard]] constexpr size_t size(char c) noexcept
	{
		return 1;
	}
	[[nodiscard]] constexpr size_t size(wchar_t w) noexcept
	{
		return 1;
	}
	[[nodiscard]] constexpr size_t size(std::string_view str) noexcept
	{
		return str.size();
	}
	[[nodiscard]] constexpr size_t size(std::wstring_view wstr) noexcept
	{
		return wstr.size();
	}

	[[nodiscard]] constexpr std::string trim(const std::string &str) noexcept
	{
		std::string copy = str;
		copy.erase(str.find_last_not_of(whitespaceDelimiters) + 1);
		return copy;
	}

	[[nodiscard]] constexpr std::string rtrim(const std::string &str) noexcept
	{
		std::string copy = str;
		copy.erase(0, str.find_first_not_of(whitespaceDelimiters));
		return copy;
	}

	[[nodiscard]] constexpr std::string ltrim(const std::string &str) noexcept
	{
		std::string copy = str;
		copy.erase(str.find_last_not_of(whitespaceDelimiters) + 1);
		copy.erase(0, str.find_first_not_of(whitespaceDelimiters));
		return copy;
	}

	[[nodiscard]] constexpr std::string toLower(const std::string &str) noexcept
	{
		std::string copy = str;
		std::transform(copy.begin(), copy.end(), copy.begin(), [](const char c)
					   { return static_cast<char>(std::tolower(c)); });
		return copy;
	}
	[[nodiscard]] constexpr std::string replace(const std::string &str, const std::string &a, const std::string &b) noexcept
	{
		std::string copy = str;
		if (!a.empty())
		{
			std::size_t pos = 0;
			while ((pos = copy.find(a, pos)) != std::string::npos)
			{
				copy.replace(pos, a.size(), b);
				pos += b.size();
			}
		}
		return copy;
	}

	void replace(std::string &str, const std::string &a, const std::string &b) noexcept
	{
		if (!a.empty())
		{
			std::size_t pos = 0;
			while ((pos = str.find(a, pos)) != std::string::npos)
			{
				str.replace(pos, a.size(), b);
				pos += b.size();
			}
		}
	}

	void toLower(std::string &str) noexcept
	{
		std::transform(str.begin(), str.end(), str.begin(), [](const char c)
					   { return static_cast<char>(std::tolower(c)); });
	}

	void trim(std::string &str) noexcept
	{
		str.erase(str.find_last_not_of(whitespaceDelimiters) + 1);
		str.erase(0, str.find_first_not_of(whitespaceDelimiters));
	}

	void ltrim(std::string &str) noexcept
	{
		str.erase(str.find_last_not_of(whitespaceDelimiters) + 1);
	}

	void rtrim(std::string &str) noexcept
	{
		str.erase(0, str.find_first_not_of(whitespaceDelimiters));
	}

	namespace
	{
		template <typename T, typename... Args>
		T concat_imp(Args &&...args)
		{
			T res;
			res.reserve((size(args) + ...));
			(res.operator+=(std::forward<Args>(args)), ...);
			return res;
		}

		template <typename T, typename U, typename V>
		T join_imp(U first, U last, V seq)
		{
			T res;
			if (first != last)
			{
				res += *first;
				++first;
			}

			for (; first != last; ++first)
			{
				res += seq;
				res += *first;
			}

			return res;
		}
	} // inline namespace impl
	template <typename... Args>
	[[nodiscard]] std::string concat(Args &&...args)
	{
		return concat_imp<std::string>(std::forward<Args>(args)...);
	}

	template <typename... Args>
	[[nodiscard]] std::wstring wconcat(Args &&...args)
	{
		return concat_imp<std::wstring>(std::forward<Args>(args)...);
	}
	template <typename T>
	[[nodiscard]] std::string join(T first, T last, std::string_view seq)
	{
		return join_imp<std::string>(first, last, seq);
	}
	template <typename T>
	[[nodiscard]] std::wstring join(T first, T last, const std::wstring &seq)
	{
		return join_imp<std::wstring>(first, last, seq);
	}

	[[nodiscard]] bool isNumeric(const std::string &str)
	{
		if (str.empty())
		{
			return false;
		}
		const char *p = str.c_str();
		char *end = nullptr;
		double result = std::strtod(p, &end);
		return end != p && *end == '\0';
	}
} // namespace plib::core::utils

#endif