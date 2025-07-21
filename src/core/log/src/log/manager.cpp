#include <memory>
#include <string>
#include <iostream>
#include "log/manager.h"
#include "log/ivyLogger.h"
#include "log/spdLogger.h"

namespace common {
namespace log {

LoggerType LogManager::s_loggerType = LoggerType::SPD;
std::shared_ptr<ILogger> LogManager::s_logger;

std::shared_ptr<ILogger> LogManager::createLoggerInstance(LoggerType type)
{
  switch (type)
  {
    case LoggerType::SPD: return std::make_shared<SpdLogger>();
    case LoggerType::IVY: return std::make_shared<IvyLogger>();
    default: return std::make_shared<SpdLogger>();
  }
}

void LogManager::init() { init(s_loggerType, LogConfig()); }

void LogManager::init(LoggerType type, LogConfig logConfig)
{
  if (s_logger) return;

// spdlog not supported under wasm platform, force to use ivylogger
#ifdef COMMON_OS_WASM
  type = LoggerType::IVY;
#endif

  s_loggerType = type;
  s_logger = createLoggerInstance(s_loggerType);
  s_logger->init(logConfig);
}

LoggerType LogManager::getLoggerType() { return s_loggerType; }

void LogManager::shutdown()
{
  if (!s_logger) return;
  s_logger->shutdown();
  s_logger.reset();
}

std::shared_ptr<ILogger>& LogManager::getLogger()
{
  if (!s_logger)
  {
    init();
  }
  return s_logger;
}

void LogManager::setLevel(LogLevel level)
{
  if (!s_logger)
  {
    init();
  }
  s_logger->setLevel(level);
}

void LogManager::flush()
{
  if (s_logger)
  {
    s_logger->flush();
  }
}

} // namespace log
} // namespace common