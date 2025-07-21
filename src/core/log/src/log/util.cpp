#include "log/util.h"

#ifdef _WIN32
#include "windows/path_utils.h"
#else
#include "linux/path_utils.h"
#endif

namespace common {
namespace log {

std::filesystem::path getLogDirectory(const std::string& configPath)
{
  // use absolute path if specificed
  if (!configPath.empty() && std::filesystem::path(configPath).is_absolute())
  {
    return configPath;
  }

  // use relative path (relative to executable)
  std::filesystem::path exePath;
#ifdef _WIN32
  exePath = common::win32::getExecutablePath();
#else
  exePath = common::linux::getExecutablePath();
#endif

  return exePath.parent_path() / "logs";
}

LogFileInfo generateLogFileName(const std::string& basePath, const std::string& appName)
{
  std::filesystem::path logDir = getLogDirectory(basePath);
  std::filesystem::create_directories(logDir);

  std::string fileName;
  auto now = std::chrono::system_clock::now();

  std::time_t t = std::chrono::system_clock::to_time_t(now);
  std::tm tm = *std::localtime(&t);
  char buffer[32];
  std::strftime(buffer, 32, "%H-%M-%S", &tm);
  fileName = appName + "_" + buffer + ".log";

  // auto now = std::chrono::system_clock::now();
  // std::string timestamp = std::format("{:%H-%M}", std::chrono::current_zone()->to_local(now));
  // fileName = appName + "_" + timestamp + ".log"; // spdlog will add date to the daily log file name, but only date, not time
  // fileName = appName + ".log";

  std::filesystem::path fullPath = logDir / fileName;
  return {fileName, fullPath.string()};
}
} // namespace log
} // namespace common