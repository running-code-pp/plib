#pragma once
#include <memory>
#include <string>
#include "global.h"
#include "log/common.h"

namespace common {

namespace log {

class ILogger;

// supported logger library
enum class LoggerType
{
  SPD, // spd logger
  IVY  // ivy custom logger
};

// manager of log system
class COMMON_EXPORT LogManager
{
  public:
  // Initialize the log system
  static void init();
  static void init(LoggerType type, LogConfig logConfig);

  // close the log system
  static void shutdown();

  // get the logger instance
  static std::shared_ptr<ILogger>& getLogger();

  // set log level
  static void setLevel(LogLevel level);

  // get the current logger type
  static LoggerType getLoggerType();

  // flush the logs to ensure all logs are written immediately
  static void flush();

  private:
  // Create logger instance factory method
  static std::shared_ptr<ILogger> createLoggerInstance(LoggerType type);

  // logger instance
  static std::shared_ptr<ILogger> s_logger;

  // current logger library type
  static LoggerType s_loggerType;
};

} // namespace log
} // namespace common
