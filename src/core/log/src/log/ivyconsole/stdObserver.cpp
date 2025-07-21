//--------------------------------------------------------------------------------------------------
// Copyright (c) ZIXEL 2024
// History:
// 2024-07-12 IVY: V0.0
//--------------------------------------------------------------------------------------------------
#include "log/ivyconsole/stdObserver.h"
#if defined(COMMON_OS_WINDOWS)
#include <windows.h>
#elif defined(COMMON_OS_UNIX) // UNIX-like platforms (Linux, macOS, etc.)
#include <unistd.h>
#elif defined(COMMON_OS_WASM)
#include <emscripten.h>
#endif

#include <cstring>

namespace common {
namespace log {

StdObserver::StdObserver() : _useColor(checkColorSupport())
{
  // sample for close trace output for std observer
  // setTrace(false);
  // sendLog(std::format("Console STD observer started. UseColor: {}", _useColor), LogLevel::INFO);
}

bool StdObserver::checkColorSupport()
{
  // check environment control variables
  // 1-enable, 0-disable, undefined-default
  const char *enableColor = std::getenv("ENABLE_COLOR");
  if (enableColor)
  {
    return enableColor[0] == '1';
  }

// default behavior when no environment variable is set
#if defined(COMMON_OS_WINDOWS)
  return true;
// #elif defined(COMMON_OS_UNIX)
#elif defined(COMMON_OS_UNIX)
  const char *term = std::getenv("TERM");
  if (term && (strstr(term, "xterm") || strstr(term, "screen") || strstr(term, "vt100") || strstr(term, "color")))
  {
    return isatty(STDOUT_FILENO);
  }
  return false;
#else
  return false;
#endif
}

// Helper function to convert LogLevel to Emscripten log level
#if defined(COMMON_OS_WASM)
int convertToEmLogLevel(LogLevel level)
{
  switch (level)
  {
    case LogLevel::FATAL:
    case LogLevel::ERR: return EM_LOG_ERROR;
    case LogLevel::WARNING: return EM_LOG_WARN;
    case LogLevel::INFO: return EM_LOG_INFO;
    case LogLevel::DEBUG:
    case LogLevel::TRACE: return EM_LOG_DEBUG;
    default: return EM_LOG_CONSOLE;
  }
}
#else

// color attributes for different log levels
#if defined(COMMON_OS_WINDOWS)
const StdObserver::ColorAttribute StdObserver::FATAL_COLOR = {
    FOREGROUND_RED | FOREGROUND_INTENSITY // highlight red
};
const StdObserver::ColorAttribute StdObserver::ERROR_COLOR = {
    FOREGROUND_RED // red
};
const StdObserver::ColorAttribute StdObserver::WARNING_COLOR = {
    FOREGROUND_RED | FOREGROUND_GREEN // yellow
};
const StdObserver::ColorAttribute StdObserver::INFO_COLOR = {
    FOREGROUND_GREEN // green
};
const StdObserver::ColorAttribute StdObserver::DEBUG_COLOR = {
    FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE // white
};
const StdObserver::ColorAttribute StdObserver::TRACE_COLOR = {
    FOREGROUND_BLUE | FOREGROUND_GREEN // cyan
};
const StdObserver::ColorAttribute StdObserver::RESET_COLOR = {
    FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE // white
};
#elif defined(COMMON_OS_UNIX)
const StdObserver::ColorAttribute StdObserver::FATAL_COLOR = {"\033[1;31m"}; // highlight red
const StdObserver::ColorAttribute StdObserver::ERROR_COLOR = {"\033[31m"};   // red
const StdObserver::ColorAttribute StdObserver::WARNING_COLOR = {"\033[33m"}; // yellow
const StdObserver::ColorAttribute StdObserver::INFO_COLOR = {"\033[32m"};    // green
const StdObserver::ColorAttribute StdObserver::DEBUG_COLOR = {"\033[37m"};   // white
const StdObserver::ColorAttribute StdObserver::TRACE_COLOR = {"\033[36m"};   // cyan
const StdObserver::ColorAttribute StdObserver::RESET_COLOR = {"\033[0m"};    // reset
#endif

void StdObserver::setConsoleColor(const ColorAttribute &attr)
{
  if (!_useColor) return;

#if defined(COMMON_OS_WINDOWS)
  ::SetConsoleTextAttribute(::GetStdHandle(STD_OUTPUT_HANDLE), attr.winAttribute);
#elif defined(COMMON_OS_UNIX)
  printf("%s", attr.ansiColor);
#endif
}
#endif

void StdObserver::sendLog(const std::string &iMsg, LogLevel level)
{
#ifdef COMMON_OS_WASM
  int emLogLevel = convertToEmLogLevel(level);
  emscripten_log(emLogLevel, "[%s] %s", logLevelToString(level), iMsg.c_str());
#else
  ColorAttribute attr = [level]() {
    switch (level)
    {
      case LogLevel::FATAL: return FATAL_COLOR;
      case LogLevel::ERR: return ERROR_COLOR;
      case LogLevel::WARNING: return WARNING_COLOR;
      case LogLevel::INFO: return INFO_COLOR;
      case LogLevel::DEBUG: return DEBUG_COLOR;
      case LogLevel::TRACE: return TRACE_COLOR;
      default: return RESET_COLOR;
    }
  }();

  setConsoleColor(attr);
  printf("%s", iMsg.c_str());
  setConsoleColor(RESET_COLOR);
#endif
}

} // namespace log
} // namespace common