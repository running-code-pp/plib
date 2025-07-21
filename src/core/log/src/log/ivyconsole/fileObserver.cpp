//--------------------------------------------------------------------------------------------------
// Copyright (c) ZIXEL 2024
// History:
// 2024-07-12 IVY: V0.0
//--------------------------------------------------------------------------------------------------
#include <iostream>
#include <format>
#include "log/common.h"
#include "log/ivyconsole/fileObserver.h"

namespace common {
namespace log {

FileObserver::FileObserver(const char *sFileName)
{
  _fStream.open(sFileName, std::ios::out);

  if (!_fStream.is_open())
  {
    std::cerr << "Cannot open log file: " << sFileName << std::endl;

    _isValid = false;
    return;
  }

  _isValid = true;
  sendLog(std::format("Console File observer started. FileName: {}", sFileName), LogLevel::INFO);
}

void FileObserver::sendLog(const std::string &sMessage, LogLevel level)
{
  if (!_isValid || !_fStream.is_open())
  {
    return; // invalid file stream
  }
  _fStream << logLevelToString(level) << ":" << sMessage;
  _fStream.flush();
}

} // namespace log
} // namespace common