#include "utils/path_util.hpp"
#include <filesystem>
#if defined(WIN32) || defined(_WIN32) || defined(_WIN32_) || defined(WIN64) || defined(_WIN64) || defined(_WIN64_)
#include <windows.h>
#include <libloaderapi.h>
#elif defined(__linux__)
#include <unistd.h>
#include <limits.h>
#include <libgen.h>
#endif


namespace plib::core::utils{
std::string get_executable_path() {
#if defined(WIN32) || defined(_WIN32) || defined(_WIN32_) || defined(WIN64) || defined(_WIN64) || defined(_WIN64_)
	char buffer[MAX_PATH];
	DWORD len = GetModuleFileNameA(NULL, buffer, MAX_PATH);
	if (len == 0 || len == MAX_PATH) return "";
	return std::string(buffer, len);
#elif defined(__linux__)
	char buffer[PATH_MAX];
	ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
	if (len == -1) return "";
	buffer[len] = '\0';
	return std::string(buffer);
#else
	return "";
#endif
}

std::string get_executable_dir() {
	std::string path = plib::core::utils::get_executable_path();
	std::filesystem::path p(path);
	return p.parent_path().string();
}

std::string get_executable_name() {
	std::string path = plib::core::utils::get_executable_path();
	std::filesystem::path p(path);
	return p.filename().string();
}

std::string get_current_dir() {
	return std::filesystem::current_path().string();
}
}