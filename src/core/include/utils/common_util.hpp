#ifndef PLIB_CORE_UTILS_COMMON_UTIL_HPP
#define PLIB_CORE_UTILS_COMMON_UTIL_HPP
#include <string>
#include <optional>
#include <cstdint>
#include <cerrno>
#include <cstdlib>
#include <cstring>

namespace plib::core::utils {
	int getRandom(int min, int max);
	std::string getCurrentTime(std::string format);
	std::optional<int64_t> ToInt64(const std::string &str, int base = 10);
}

#endif // PLIB_CORE_UTILS_COMMON_UTIL_HPP