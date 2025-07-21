#include <filesystem>
#include <format>
#include <atomic>
#include <mutex>
#include "log/ivyLogger.h"
#include "log/util.h"
#include "log/ivyconsole/ivyconsole.h"
#include "log/ivyconsole/stdObserver.h"
#include "log/ivyconsole/fileObserver.h"

namespace {
std::atomic<int> g_loggerInstanceCount{0}; // trace IvyLogger instance number, decide destruct of Console
std::mutex g_consoleMutex;                 // mutex to protect destruction of Console
} // namespace

namespace common {
namespace log {
IvyLogger::IvyLogger() : m_logger(nullptr), m_currentLevel(LogLevel::INFO), m_initialized(false) { g_loggerInstanceCount++; }

IvyLogger::~IvyLogger()
{
  shutdown();

  int expected = g_loggerInstanceCount.fetch_sub(1) - 1;
  if (expected <= 0)
  {
    std::lock_guard<std::mutex> lock(g_consoleMutex);
    // check again to prevent other thread have executed destruct()
    if (expected <= 0)
    {
      Console::destruct();
      g_loggerInstanceCount.store(0); // prevent negative value
    }
  }
}

void IvyLogger::init(const LogConfig& config)
{
  if (m_initialized) return;

  m_logger = std::shared_ptr<Console>(&Console::Instance(), [](Console*) {
    // empty, do nothing when deleted
    // because Console is singleton, shouldn't be deleted by shared_ptr
  });

  m_logger->setLogLevel(config.logLevel);
  m_currentLevel = config.logLevel;

  // std output
  if (config.enableConsole)
  {
    auto stdObserver = std::make_shared<StdObserver>();
    m_logger->attachObserver(std::dynamic_pointer_cast<Observer>(stdObserver));
  }

  // file output
#ifndef __EMSCRIPTEN__
  if (config.enableFile)
  {
    LogFileInfo fileInfo = generateLogFileName(config.logPath, config.appName);
    auto fileObserver = std::make_shared<FileObserver>(fileInfo.fullPath.c_str());
    if (fileObserver->isValid())
    {
      m_logger->attachObserver(std::dynamic_pointer_cast<Observer>(fileObserver));
    }
  }
#else
  // WASM environments doesn't support file logging
  if (config.enableFile)
  {
    if (config.enableConsole)
    {
      log(LogLevel::WARNING, "ivyLogger.cpp", __LINE__, __FUNCTION__, ("File logging is not supported in WASM environment"));
    }
  }
#endif

  m_initialized = true;
  log(LogLevel::INFO, "ivyLogger.cpp", __LINE__, __FUNCTION__, ("IvyLogger initialized. Config: " + config.toString()).c_str());
}

void IvyLogger::shutdown()
{
  if (!m_initialized) return;

  if (m_logger)
  {
    log(LogLevel::INFO, __FILE__, __LINE__, __FUNCTION__, "IvyLogger shutting down.");
    flush();
    m_logger.reset();
  }

  m_initialized = false;
}

void IvyLogger::setLevel(LogLevel level)
{
  m_currentLevel = level;

  if (m_logger)
  {
    m_logger->setLogLevel(level);
  }
}

LogLevel IvyLogger::getLevel() const { return m_currentLevel; }

void IvyLogger::log(LogLevel level, const char* file, int line, const char* func, const char* msg)
{
  if (!m_initialized || !m_logger) return;
  m_logger->logWithCtx(level, file, line, func, msg);
}

// flush logs
void IvyLogger::flush()
{
  // ivy logger have no flush action, do nothing
}

} // namespace log
} // namespace common