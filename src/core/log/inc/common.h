#pragma once
#include <string>
#include <sstream>

namespace common {
namespace log {

/** level of log*/
enum class LogLevel
{
  TRACE,
  DEBUG,
  INFO,
  WARNING,
  ERR,
  FATAL,
  OFF
};

inline const char* logLevelToString(LogLevel level)
{
  switch (level)
  {
    case LogLevel::FATAL: return "FAT";
    case LogLevel::ERR: return "ERR";
    case LogLevel::WARNING: return "WAR";
    case LogLevel::INFO: return "INF";
    case LogLevel::DEBUG: return "DBG";
    case LogLevel::TRACE: return "TAC";
    default: return "UNKNOWN";
  }
}

struct LogConfig
{
  std::string appName = "zcad-server";  // app name, used as log file name
  std::string logPath = "";             // log file path, empty means use default path, which is log under executable file folder
  LogLevel logLevel = LogLevel::DEBUG;  // log level enabled
  size_t maxFileSize = 5 * 1024 * 1024; // max log file size, 5MB default
  size_t maxFiles = 10;                 // max retain files number
  bool enableConsole = true;            // enable console output
  bool enableFile = true;               // enable file output
  bool enableColor = true;              // enable color option
  bool enableAsync = true;              // enable async logging

  std::string toString() const
  {
    std::ostringstream oss;
    oss << "[ ";
    oss << "\"appName\": \"" << appName << "\", ";
    oss << "\"logPath\": \"" << logPath << "\", ";
    oss << "\"logLevel\": \"" << logLevelToString(logLevel) << "\", ";
    oss << "\"maxFileSize\": " << maxFileSize << ", ";
    oss << "\"maxFiles\": " << maxFiles << ", ";
    oss << "\"enableConsole\": " << (enableConsole ? "true" : "false") << ", ";
    oss << "\"enableFile\": " << (enableFile ? "true" : "false") << ", ";
    oss << "\"enableColor\": " << (enableColor ? "true" : "false") << ", ";
    oss << "\"enableAsync\": " << (enableAsync ? "true" : "false");
    oss << " ]";
    return oss.str();
  }
};

} // namespace log
} // namespace common