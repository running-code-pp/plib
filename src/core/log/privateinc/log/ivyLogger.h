#pragma once

#include <memory>
#include "log/ilogger.h"
#include "log/common.h"
#include "log/ivyconsole/ivyconsole.h"

namespace common {
namespace log {

/**
 * @brief IvyLogger - Ilogger based on IvyConsole
 *
 */
class IvyLogger : public ILogger
{
  public:
  IvyLogger();
  virtual ~IvyLogger() override;

  void init(const LogConfig& config) override;

  void shutdown() override;

  void setLevel(LogLevel level) override;

  LogLevel getLevel() const override;

  void log(LogLevel level, const char* file, int line, const char* func, const char* msg) override;

  void flush() override;

  private:
  std::shared_ptr<Console> m_logger;
  LogLevel m_currentLevel;
  bool m_initialized;

  bool supportsDirectFormatting() const override { return false; }
};

} // namespace log
} // namespace common