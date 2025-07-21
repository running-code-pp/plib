#pragma once
#include <string>
#include <filesystem>

namespace common {
namespace log {

struct LogFileInfo
{
  std::string fileName;
  std::string fullPath;
};

std::filesystem::path getLogDirectory(const std::string& configPath);
LogFileInfo generateLogFileName(const std::string& basePath, const std::string& appName);

} // namespace log
} // namespace common