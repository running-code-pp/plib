#ifndef PLIB_CORE_LOG_LOG_HPP
#define PLIB_CORE_LOG_LOG_HPP
#include "log/manager.hpp"
#include "log/logger.hpp"

#define LOG_EXTRACT_FILENAME(path) extractFilename(path)    // extract filename from path
#define LOG_CURRENT_FILENAME LOG_EXTRACT_FILENAME(__FILE__) // current filename

#define LOG_TRACE(msg, ...) \
plib::core::log::LogManager::getLogger()->logWithFmt(plib::core::log::LogLevel::TRACE, LOG_CURRENT_FILENAME, __LINE__, __func__, msg, ##__VA_ARGS__)
#define LOG_DEBUG(msg, ...) \
plib::core::log::LogManager::getLogger()->logWithFmt(plib::core::log::LogLevel::DEBUG, LOG_CURRENT_FILENAME, __LINE__, __func__, msg, ##__VA_ARGS__)
#define LOG_INFO(msg, ...) \
plib::core::log::LogManager::getLogger()->logWithFmt(plib::core::log::LogLevel::INFO, LOG_CURRENT_FILENAME, __LINE__, __func__, msg, ##__VA_ARGS__)
#define LOG_WARN(msg, ...) \
plib::core::log::LogManager::getLogger()->logWithFmt(plib::core::log::LogLevel::WARNING, LOG_CURRENT_FILENAME, __LINE__, __func__, msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...) \
plib::core::log::LogManager::getLogger()->logWithFmt(plib::core::log::LogLevel::ERR, LOG_CURRENT_FILENAME, __LINE__, __func__, msg, ##__VA_ARGS__)
#define LOG_FATAL(msg, ...) \
plib::core::log::LogManager::getLogger()->logWithFmt(plib::core::log::LogLevel::FATAL, LOG_CURRENT_FILENAME, __LINE__, __func__, msg, ##__VA_ARGS__)

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
#endif // PLIB_CORE_LOG_LOG_HPP