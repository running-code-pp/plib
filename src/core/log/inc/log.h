#pragma once
#include "log/manager.h"
#include "log/ilogger.h"

#define LOG_EXTRACT_FILENAME(path) extractFilename(path)    // extract filename from path
#define LOG_CURRENT_FILENAME LOG_EXTRACT_FILENAME(__FILE__) // current filename

#define LOG_TRACE(msg, ...) \
  ::common::log::LogManager::getLogger()->logWithFmt(::common::log::LogLevel::TRACE, LOG_CURRENT_FILENAME, __LINE__, __func__, msg, ##__VA_ARGS__)
#define LOG_DEBUG(msg, ...) \
  ::common::log::LogManager::getLogger()->logWithFmt(::common::log::LogLevel::DEBUG, LOG_CURRENT_FILENAME, __LINE__, __func__, msg, ##__VA_ARGS__)
#define LOG_INFO(msg, ...) \
  ::common::log::LogManager::getLogger()->logWithFmt(::common::log::LogLevel::INFO, LOG_CURRENT_FILENAME, __LINE__, __func__, msg, ##__VA_ARGS__)
#define LOG_WARNING(msg, ...) \
  ::common::log::LogManager::getLogger()->logWithFmt(common::log::LogLevel::WARNING, LOG_CURRENT_FILENAME, __LINE__, __func__, msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...) \
  ::common::log::LogManager::getLogger()->logWithFmt(::common::log::LogLevel::ERR, LOG_CURRENT_FILENAME, __LINE__, __func__, msg, ##__VA_ARGS__)
#define LOG_FATAL(msg, ...) \
  ::common::log::LogManager::getLogger()->logWithFmt(::common::log::LogLevel::FATAL, LOG_CURRENT_FILENAME, __LINE__, __func__, msg, ##__VA_ARGS__)

#define LOG_FLUSH() ::common::log::LogManager::flush()

inline const char* extractFilename(const char* path)
{
  if (!path || !*path) return path; // handle NULL or empty string

  const char* lastSlash = nullptr;
  const char* lastBackslash = nullptr;

  for (const char* p = path; *p; ++p)
  {
    if (*p == '/')
      lastSlash = p;
    else if (*p == '\\')
      lastBackslash = p;
  }

  const char* lastSeparator = lastSlash > lastBackslash ? lastSlash : lastBackslash;
  return lastSeparator ? lastSeparator + 1 : path;
}
