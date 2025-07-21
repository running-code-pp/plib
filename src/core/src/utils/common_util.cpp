#include "utils/common_util.hpp"
#include<chrono>
namespace plib::core::utils {
	int getRandomInt(int min, int max) {
		int mmax = max > min ? max : min;
		int mmin = min > max ? max : min;
		return rand() % (mmax - mmin + 1) + mmin;
	}

	std::string getCurrentTime(std::string format) {
		auto now = std::chrono::system_clock::now();
		auto in_time_t = std::chrono::system_clock::to_time_t(now);
		std::stringstream ss;
#if defined(WIN32) || defined(_WIN32) || defined(_WIN32_) || defined(WIN64) || defined(_WIN64) || defined(_WIN64_)
		struct tm buf;
		localtime_s(&buf, &in_time_t);
		ss << std::put_time(&buf, format.c_str());
#else
		struct tm buf;
		localtime_r(&in_time_t, &buf);
		ss << std::put_time(&buf, format.c_str());
#endif
		return ss.str();
	}
}