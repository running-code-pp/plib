#ifndef PLIB_CORE_UTILS_COMMON_UTIL_HPP
#define PLIB_CORE_UTILS_COMMON_UTIL_HPP
#include <string>
namespace plib::core::utils {
	int getRandom(int min, int max);
	std::string getCurrentTime(std::string format);
}

#endif // PLIB_CORE_UTILS_COMMON_UTIL_HPP