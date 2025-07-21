#pragma once
#include <string>
#include <memory>
#include <format>
#include "log/common.h"

namespace detail {
// Safe formatting function that handles all potential formatting errors
template <typename... Args>
std::string safeFormat(const char* fmt, Args&&... args)
{
  if constexpr (sizeof...(args) == 0)
  {
    return fmt;
  }

  try
  {
    return std::vformat(fmt, std::make_format_args(args...));
  }
  catch (const std::format_error& e)
  {
    // Provide friendly error message
    std::string errorMsg = std::string(fmt) + " [Format error: " + e.what() + "]";

    // Optional: Try to include parameter values (limited by type conversions)
    try
    {
      std::ostringstream oss;
      oss << " [Args:";
      int dummy[] = {0, ((oss << ' ' << args), 0)...};
      (void)dummy;
      oss << "]";
      errorMsg += oss.str();
    }
    catch (...)
    {
      errorMsg += " [Args cannot be displayed]";
    }

    return errorMsg;
  }
  catch (const std::exception& e)
  {
    return std::string(fmt) + " [Exception: " + e.what() + "]";
  }
  catch (...)
  {
    return std::string(fmt) + " [Unknown exception]";
  }
}
} // namespace detail

namespace common {
namespace log {

// unified interface for different logger library
class ILogger
{
  public:
  virtual ~ILogger() = default;

  // Initialize the logging system (overloaded method for configuration support)
  virtual void init(const LogConfig& config) = 0;

  // Shutdown the logging system
  virtual void shutdown() = 0;

  // Set the logging level
  virtual void setLevel(LogLevel level) = 0;

  // Get the current logging level
  virtual LogLevel getLevel() const = 0;

  // record a log message
  virtual void log(LogLevel level, const char* file, int line, const char* func, const char* msg) = 0;

  // record a log message with format params
  template <typename... Args>
  void logWithFmt(LogLevel level, const char* file, int line, const char* func, const char* msg, Args&&... args)
  {
    std::string formattedMsg;
    if constexpr (sizeof...(args) > 0)
    {
      try
      {
        formattedMsg = std::vformat(msg, std::make_format_args(args...));
      }
      catch (const std::exception& e)
      {
        formattedMsg = std::string(msg) + " [format error: " + e.what() + "]";
      }
    }
    else
    {
      formattedMsg = msg;
    }

    log(level, file, line, func, formattedMsg.c_str());
  }

  // flush logger buffer, usable when logger support async logging
  // usually no need to call
  // called when unexpected crash or program exit
  virtual void flush() = 0;

  private:
  // identify if the logger library supports direct formatting
  virtual bool supportsDirectFormatting() const { return false; }
};

} // namespace log
} // namespace common