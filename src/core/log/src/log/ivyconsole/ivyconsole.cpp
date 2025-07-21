//--------------------------------------------------------------------------------------------------
// Copyright (c) ZIXEL 2024
// History:
// 2024-07-12 IVY: V0.0
//--------------------------------------------------------------------------------------------------
#include <thread>
#include "log/ivyconsole/ivyconsole.h"
#include "global.h"

#if defined(COMMON_OS_WASM)
#include <emscripten.h>
#endif

namespace common {
namespace log {

Console::Console()
    : _bVerbose(true),
      connectionMode(Direct)
#ifdef IVY_DEBUG
      ,
      _logLevel(LogLevel::TRACE)
#else
      ,
      _logLevel(LogLevel::DEBUG)
#endif
{
}

Console::~Console() { _aclObservers.clear(); }

Console *Console::_pcSingleton = nullptr;

/**
 *  delete
 */
void Console::destruct()
{
  // not initialized or double destructed!
  if (nullptr == _pcSingleton) return;
  delete _pcSingleton;
  _pcSingleton = nullptr;
}

/**
 *  init
 */
Console &Console::Instance()
{
  // not initialized?
  if (!_pcSingleton)
  {
    _pcSingleton = new Console();
  }
  return *_pcSingleton;
}

/**
 *  sets the console in a special mode
 */
void Console::setConsoleMode(ConsoleMode iMode)
{
  if (iMode == ConsoleMode::Verbose)
  {
    _bVerbose = true;
  }
}

/**
 *  unsets the console from a special mode
 */
void Console::unsetConsoleMode(ConsoleMode iMode)
{
  if (iMode == ConsoleMode::Verbose)
  {
    _bVerbose = false;
  }
}

void Console::setConnectionMode(ConnectionMode iMode) { connectionMode = iMode; }

/** Attaches an Observer to console
 *  Use this method to attach a Observer derived class to
 *  the console. After the observer is attached all messages will also
 *  be forwarded to it.
 *  @see Observer
 */
void Console::attachObserver(std::shared_ptr<Observer> ipcObserver)
{
  // double insert !!
  if (_aclObservers.find(ipcObserver) != _aclObservers.end()) return;
  _aclObservers.insert(ipcObserver);
}

/** Detaches an Observer from console
 *  Use this method to detach a Observer derived class.
 *  After detaching you can destruct the Observer or reinsert it later.
 *  @see Observer
 */
void Console::detachObserver(std::shared_ptr<Observer> ipcObserver) { _aclObservers.erase(ipcObserver); }

void Console::sendLogToObserver(LogLevel level, const std::string &iMsg)
{
  for (std::set<std::shared_ptr<Observer>>::iterator Iter = _aclObservers.begin(); Iter != _aclObservers.end(); ++Iter)
  {
    if ((*Iter)->isActive(level))
    {
      (*Iter)->sendLog(iMsg, level); // send string to the listener
    }
  }
}

std::string Console::formatLog(const std::string &msg, const char *file, int line, const char *func, LogLevel level)
{
  std::ostringstream oss;
  oss << "[L]" << getTimestamp() << "|" << logLevelToString(level) << "|" << std::this_thread::get_id() << "|" << file << "|" << line << "|" << func << "|"
      << msg << "|\n";
  return oss.str();
}

std::string Console::getTimestamp()
{
  static constexpr size_t BUFFER_SIZE = 32;

  std::array<char, BUFFER_SIZE> buffer;

  auto now = std::chrono::system_clock::now();
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

  auto timer = std::chrono::system_clock::to_time_t(now);

#if defined(COMMON_OS_WINDOWS)
  std::tm bt;
  localtime_s(&bt, &timer);
#elif defined(COMMON_OS_WASM)
  std::tm bt;

  // Fetch JavaScript Timestamp
  double jsTime = EM_ASM_DOUBLE({
    var d = new Date();
    return d.getTime();
  });

  // Convert to local time
  time_t jsTimer = static_cast<time_t>(jsTime / 1000);
  bt = *std::gmtime(&jsTimer); // Use GME time in WASM

  // adjust milliseconds
  ms = std::chrono::milliseconds(static_cast<int64_t>(jsTime) % 1000);
#else // Linux, MacOS
  std::tm bt;
  localtime_r(&timer, &bt);
#endif

  int written = std::snprintf(buffer.data(), BUFFER_SIZE, "%04d-%02d-%02d %02d:%02d:%02d.%03d", bt.tm_year + 1900, bt.tm_mon + 1, bt.tm_mday, bt.tm_hour,
                              bt.tm_min, bt.tm_sec, static_cast<int>(ms.count()));

  // ensure the string is written correctly
  if (written > 0 && written < static_cast<int>(BUFFER_SIZE))
  {
    return std::string(buffer.data(), written);
  }

  // return a default string when error occurs
  return "0000-00-00 00:00:00.000";
}

} // namespace log
} // namespace common