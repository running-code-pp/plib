
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <filesystem>
#include <vector>
#include "log/spdLogger.h"
#include "log/common.h"
#include "log/util.h"
#include <thread>
#include <sstream>

namespace common {
namespace log {

SpdLogger::SpdLogger() : m_logger(nullptr), m_currentLevel(LogLevel::INFO), m_initialized(false) {}

SpdLogger::~SpdLogger() { shutdown(); }

void SpdLogger::init(const LogConfig& config)
{
  if (m_initialized) return;

  std::vector<spdlog::sink_ptr> sinks;
  // add console output
  if (config.enableConsole)
  {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_pattern("[L]%Y-%m-%d %H:%M:%S.%e|%^%l%$|%t|%P|%s|%#|%!|%v");
    sinks.push_back(console_sink);
  }

  // add file output
#ifndef __EMSCRIPTEN__
  if (config.enableFile)
  {
    LogFileInfo fileInfo = generateLogFileName(config.logPath, config.appName);
    auto daily_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(fileInfo.fullPath, 0, 0, false, (uint16_t)config.maxFiles);
    daily_sink->set_pattern("[L]%Y-%m-%d %H:%M:%S.%e|%l|%t|%P|%s|%#|%!|%v");
    sinks.push_back(daily_sink);
  }
#else
  // WASM environments doesn't support file logging
  if (config.enableFile)
  {
    if (config.enableConsole)
    {
      spdlog::warn("File logging is not supported in WASM environment");
    }
  }
#endif

  if (config.enableAsync)
  {
    // output log asynchronously
    spdlog::init_thread_pool(8192, 1);
    m_logger = std::make_shared<spdlog::async_logger>("spdlogger", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
  }
  else
  {
    // output log synchronously
    m_logger = std::make_shared<spdlog::logger>("spdlogger", sinks.begin(), sinks.end());
  }

  m_currentLevel = config.logLevel;
  m_logger->set_level(convertToSpdLogLevel(m_currentLevel));
  m_logger->flush_on(spdlog::level::err);
  spdlog::set_default_logger(m_logger);

  m_initialized = true;
  SPDLOG_LOGGER_INFO(m_logger, "SpdLogger initialized. Config: {}", config.toString());
}

void SpdLogger::shutdown()
{
  if (m_initialized)
  {
    // if (m_logger)
    // {
    // m_logger->flush();
    // }
    // spdlog::shutdown();
    m_logger.reset();
    m_initialized = false;
  }
}

void SpdLogger::setLevel(LogLevel level)
{
  m_currentLevel = level;
  if (m_logger)
  {
    m_logger->set_level(convertToSpdLogLevel(level));
  }
}

LogLevel SpdLogger::getLevel() const { return m_currentLevel; }

void SpdLogger::log(LogLevel level, const char* file, int line, const char* func, const char* msg)
{
  if (!m_logger)
  {
    return;
  }

  spdlog::source_loc loc{file, line, func};

  switch (level)
  {
    case LogLevel::TRACE: m_logger->log(loc, spdlog::level::trace, msg); break;
    case LogLevel::DEBUG: m_logger->log(loc, spdlog::level::debug, msg); break;
    case LogLevel::INFO: m_logger->log(loc, spdlog::level::info, msg); break;
    case LogLevel::WARNING: m_logger->log(loc, spdlog::level::warn, msg); break;
    case LogLevel::ERR: m_logger->log(loc, spdlog::level::err, msg); break;
    case LogLevel::FATAL: m_logger->log(loc, spdlog::level::critical, msg); break;
    default: m_logger->log(loc, spdlog::level::info, msg); break;
  }
}

void SpdLogger::flush()
{
  if (m_initialized && m_logger)
  {
    m_logger->flush();
  }
}

spdlog::level::level_enum SpdLogger::convertToSpdLogLevel(LogLevel level) const
{
  switch (level)
  {
    case LogLevel::TRACE: return spdlog::level::trace;
    case LogLevel::DEBUG: return spdlog::level::debug;
    case LogLevel::INFO: return spdlog::level::info;
    case LogLevel::WARNING: return spdlog::level::warn;
    case LogLevel::ERR: return spdlog::level::err;
    case LogLevel::FATAL: return spdlog::level::critical;
    case LogLevel::OFF: return spdlog::level::off;
    default: return spdlog::level::info;
  }
}

LogLevel SpdLogger::convertFromSpdLogLevel(spdlog::level::level_enum level) const
{
  switch (level)
  {
    case spdlog::level::trace: return LogLevel::TRACE;
    case spdlog::level::debug: return LogLevel::DEBUG;
    case spdlog::level::info: return LogLevel::INFO;
    case spdlog::level::warn: return LogLevel::WARNING;
    case spdlog::level::err: return LogLevel::ERR;
    case spdlog::level::critical: return LogLevel::FATAL;
    case spdlog::level::off: return LogLevel::OFF;
    default: return LogLevel::INFO;
  }
}

} // namespace log

} // namespace common