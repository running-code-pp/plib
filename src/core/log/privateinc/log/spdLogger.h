#pragma once
#include <spdlog/spdlog.h>
#include <string>
#include <memory>
#include "log/ilogger.h"

namespace spdlog {
class logger;
}

namespace common {
namespace log {

// spdlog 实现
class SpdLogger : public ILogger
{
  public:
  SpdLogger();
  virtual ~SpdLogger() override;

  void init(const LogConfig& config) override;
  void shutdown() override;
  void setLevel(LogLevel level) override;
  LogLevel getLevel() const override;

  void log(LogLevel level, const char* file, int line, const char* func, const char* msg) override;

  template <typename... Args>
  void logWithFmt(LogLevel level, const char* file, int line, const char* func, const char* msg, Args&&... args)
  {
    if (!m_logger)
    {
      return;
    }

    spdlog::source_loc loc{file, line, func};

    switch (level)
    {
      case LogLevel::TRACE: m_logger->log(loc, spdlog::level::trace, msg, std::forward<Args>(args)...); break;
      case LogLevel::DEBUG: m_logger->log(loc, spdlog::level::debug, msg, std::forward<Args>(args)...); break;
      case LogLevel::INFO: m_logger->log(loc, spdlog::level::info, msg, std::forward<Args>(args)...); break;
      case LogLevel::WARNING: m_logger->log(loc, spdlog::level::warn, msg, std::forward<Args>(args)...); break;
      case LogLevel::ERR: m_logger->log(loc, spdlog::level::err, msg, std::forward<Args>(args)...); break;
      case LogLevel::FATAL: m_logger->log(loc, spdlog::level::critical, msg, std::forward<Args>(args)...); break;
      default: m_logger->log(loc, spdlog::level::info, msg, std::forward<Args>(args)...); break;
    }
  }

  void flush() override;

  private:
  // convert log level
  spdlog::level::level_enum convertToSpdLogLevel(LogLevel level) const;
  LogLevel convertFromSpdLogLevel(spdlog::level::level_enum level) const;
  bool supportsDirectFormatting() const override { return true; }

  std::shared_ptr<spdlog::logger> m_logger; // spdlog instance
  LogLevel m_currentLevel;                  // output log level
  bool m_initialized;                       // true if initialized
};

} // namespace log
} // namespace common