//--------------------------------------------------------------------------------------------------
// Copyright (c) ZIXEL 2024
// History:
// 2024-07-12 IVY: V0.0
//--------------------------------------------------------------------------------------------------
#ifndef ivyconsole_H_
#define ivyconsole_H_

#include <array>
#include <chrono>
#include <map>
#include <set>
#include <string>
#include <sstream>
#include <cstring>
#include <format>
#include <string_view>

#include "log/common.h"
#include "log/ivyconsole/observer.h"

namespace common {
namespace log {

/** The console class
 *  This class manage all the stdio stuff, mainly for log of all levels.
 *  The incoming Messages are distributed with the
 *  IVYConsoleObserver. The IVYConsole class itself makes no IO, it's more like a manager.
 *  \par
 *  Console is a singleton! That means you can access the only
 *  instance of the class from every where in c++ by simply using:
 *  \code
 *  #include <ivy/Console.h>
 *  ivy::Console().Log("Stage: %d",i);
 *  \endcode
 *  \par
 *  @see ConsoleObserver
 */
class Console
{
  public:
  // exported functions goes here +++++++++++++++++++++++++++++++++++++++

  /** Sends a log message of all level.
      Notification can be direct or via queue.
  */
  template <LogLevel, typename... Args>
  inline void send(const char* pMsg, Args&&... args);
  template <typename... Args>
  inline std::string formatString(const char* format, Args&&... args);

  /// Prints a Message
  template <typename... Args>
  inline void message(const char* pMsg, Args&&... args);

  template <typename... Args>
  inline void message(const std::string& pMsg, Args&&... args);
  /// Prints a warning Message
  template <typename... Args>
  inline void warning(const char* pMsg, Args&&... args);
  /// Prints a error Message
  template <typename... Args>
  inline void error(const char* pMsg, Args&&... args);
  /// Prints a log Message
  template <typename... Args>
  inline void log(const char* pMsg, Args&&... args);
  /// Prints a FATAL Message
  template <typename... Args>
  inline void critical(const char* pMsg, Args&&... args);

  /** Send message of type logStyle with environment context */
  template <LogLevel level, typename... Args>
  inline void sendWithCtx(const char* file, int line, const char* func, const char* msg, Args&&... args);
  template <typename... Args>
  inline void logWithCtx(LogLevel level, const char* file, int line, const char* func, const char* msg, Args&&... args);

  // Notify a message directly to observers
  template <LogLevel>
  inline void notify(const std::string& iMsg);

  /// Attaches an Observer to IVYConsole
  void attachObserver(std::shared_ptr<Observer> ipcObserver);
  /// Detaches an Observer from IVYConsole
  void detachObserver(std::shared_ptr<Observer> ipcObserver);

  /// enumeration for the console modes
  enum class ConsoleMode
  {
    Verbose = 1,
  };
  enum ConnectionMode
  {
    Direct = 0, // The log message is immediately written to the specified output (such as a file, console, etc.). This method is executed synchronously when
                // sending logs.
    Queued = 1 // Messages are placed in a queue and processed asynchronously. This method does not immediately record logs, but stores them in memory for later
               // processing.
  };

  enum Ivy_ConsoleMsgType
  {
    MsgType_Txt = 1,
    MsgType_Log = 2, // ConsoleObserverStd sends this and higher to stderr
    MsgType_Wrn = 4,
    MsgType_Err = 8,
    MsgType_Critical = 16, // Special message to notify critical information
  };
  /// Change mode
  void setConsoleMode(ConsoleMode iMode);
  /// Change mode
  void unsetConsoleMode(ConsoleMode iMode);

  void setConnectionMode(ConnectionMode iMode);

  void setLogLevel(LogLevel level) { _logLevel = level; }

  LogLevel getLogLevel() const { return _logLevel; }

  static void destruct();
  /// singleton
  static Console& Instance();

  inline constexpr Ivy_ConsoleMsgType getConsoleMsg(LogLevel iStyle);

  protected:
  bool _bVerbose;
  ConnectionMode connectionMode;

  // Singleton!
  Console();
  virtual ~Console();

  private:
  static Console* _pcSingleton;
  std::set<std::shared_ptr<Observer>> _aclObservers; // observer list
  LogLevel _logLevel;                                // global active log level control, logs smaller than the level will be ignored

  void sendLogToObserver(LogLevel level, const std::string& iMsg);
  std::string getTimestamp();
  std::string formatLog(const std::string& msg, const char* file, int line, const char* func, LogLevel category);
};

} // namespace log
} // namespace common

#include "ivyconsole.inl"

#endif
