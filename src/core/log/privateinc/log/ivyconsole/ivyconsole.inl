#ifndef IVYCONSOLE_INL_
#define IVYCONSOLE_INL_
namespace common {
namespace log {

// static util function

/** Access to the Console
 *  This method is used to gain access to the one and only instance of
 *  the Console class.
 */
inline Console& console() { return Console::Instance(); }



inline constexpr Console::Ivy_ConsoleMsgType Console::getConsoleMsg(LogLevel iStyle)
{
  constexpr std::array msgTypes{
      // In order of logStyle
      Ivy_ConsoleMsgType::MsgType_Wrn, Ivy_ConsoleMsgType::MsgType_Txt,      Ivy_ConsoleMsgType::MsgType_Err,
      Ivy_ConsoleMsgType::MsgType_Log, Ivy_ConsoleMsgType::MsgType_Critical,
  };

  return msgTypes.at(static_cast<std::size_t>(iStyle));
}

/** Prints a Message
 *  This method issues a Message.
 *  Messages are used to show some non vital information. That means when
 *  IVY is running in GUI mode a Message appears on the status bar.
 *  In console mode a message is printed to the console.
 *  \par
 *  You can use a printf like interface like:
 *  \code
 *  console().message("Doing something important %d times\n",i);
 *  \endcode
 *  @see Warning
 *  @see Error
 *  @see Log
 *  @see Critical
 */
template <typename... Args>
inline void Console::message(const char* pMsg, Args&&... args)
{
  if (static_cast<int>(_logLevel) > static_cast<int>(LogLevel::DEBUG)) return;
  send<LogLevel::DEBUG>(pMsg, std::forward<Args>(args)...);
}

template <typename... Args>
inline void Console::message(const std::string& pMsg, Args&&... args)
{
  message(pMsg.c_str(), std::forward<Args>(args)...);
}

template <typename... Args>
inline void Console::warning(const char* pMsg, Args&&... args)
{
  if (static_cast<int>(_logLevel) > static_cast<int>(LogLevel::WARNING)) return;
  send<LogLevel::WARNING>(pMsg, std::forward<Args>(args)...);
}

template <typename... Args>
inline void Console::error(const char* pMsg, Args&&... args)
{
  if (static_cast<int>(_logLevel) > static_cast<int>(LogLevel::ERR)) return;
  send<LogLevel::ERR>(pMsg, std::forward<Args>(args)...);
}

template <typename... Args>
inline void Console::critical(const char* pMsg, Args&&... args)
{
  send<LogLevel::FATAL>(pMsg, std::forward<Args>(args)...);
}

template <typename... Args>
inline void Console::log(const char* pMsg, Args&&... args)
{
  if (static_cast<int>(_logLevel) > static_cast<int>(LogLevel::INFO)) return;
  send<LogLevel::INFO>(pMsg, std::forward<Args>(args)...);
}

/**
 * @brief Format log with environment context information inserted.
 */
template <typename... Args>
inline void Console::logWithCtx(LogLevel level, const char* file, int line, const char* func, const char* msg, Args&&... args)
{
  if (static_cast<int>(_logLevel) > static_cast<int>(level)) return;
  switch (level)
  {
    case LogLevel::FATAL: sendWithCtx<LogLevel::FATAL>(file, line, func, msg, std::forward<Args>(args)...); break;
    case LogLevel::WARNING: sendWithCtx<LogLevel::WARNING>(file, line, func, msg, std::forward<Args>(args)...); break;
    case LogLevel::ERR: sendWithCtx<LogLevel::ERR>(file, line, func, msg, std::forward<Args>(args)...); break;
    case LogLevel::INFO: sendWithCtx<LogLevel::INFO>(file, line, func, msg, std::forward<Args>(args)...); break;
    case LogLevel::DEBUG: sendWithCtx<LogLevel::DEBUG>(file, line, func, msg, std::forward<Args>(args)...); break;
    case LogLevel::TRACE: sendWithCtx<LogLevel::TRACE>(file, line, func, msg, std::forward<Args>(args)...); break;
    default: sendWithCtx<LogLevel::INFO>(file, line, func, msg, std::forward<Args>(args)...); break;
  }
}

/**
 *@ brief Format string, insert the specified parameters into the formatted string.
 *
 *This function accepts a format string and a variable number of parameters, and replaces the parameters with placeholders in the format string.
 *Placeholders in format strings start with '%'. If you need to display the '%' character, you can use '%%'.
 *
 *@ tparam Args parameter package, representing any number of parameter types.
 *@ param format string, where '%' will be replaced by subsequent parameters.
 *The variable number of parameters in @ param args corresponds in order to placeholders in the format string.
 *@ return returns a formatted string.
 *
 *If the number of placeholders in the format string does not match the number of parameters provided, @ throws std:: runtime_ error.
 *If more parameters are provided than placeholders, throw 'Too many arguments provided for format string'.
 *If the number of parameters provided is less than the placeholder, throw 'Insufficient arguments provided for format string'.
 */
template <typename... Args>
inline std::string Console::formatString(const char* format, Args&&... args)
{
  std::ostringstream oss;
  const char* traverse = format;
  bool allArgsUsed = false; // whether all parameters have been used

  auto addArgToStream = [&](auto&& arg) {
    if (allArgsUsed) return; // all % have been processed, return directly

    while (*traverse != '\0')
    {
      if (*traverse == '%' && *(traverse + 1) != '%')
      {
        oss << arg;
        traverse += 2;
        return;
      }
      else if (*traverse == '%' && *(traverse + 1) == '%')
      {
        // Handle escaped %%
        oss << '%';
        traverse += 2;
      }
      else
      {
        oss << *traverse++;
      }
    }

    // Reached the end of the string, but there are still unused parameters
    allArgsUsed = true;
  };

  // Process each parameter
  ([&] { addArgToStream(std::forward<Args>(args)); }(), ...);

  // Process remaining format string
  while (*traverse != '\0')
  {
    if (*traverse == '%' && *(traverse + 1) != '%')
    {
      // Format string has % but no corresponding parameter, output % as is
      oss << '%';
      traverse++;
    }
    else if (*traverse == '%' && *(traverse + 1) == '%')
    {
      // Handle escaped %%
      oss << '%';
      traverse += 2;
    }
    else
    {
      oss << *traverse++;
    }
  }
  oss << "\n";

  return oss.str();
}

template <LogLevel level, typename... Args>
inline void Console::send(const char* pMsg, Args&&... args)
{
  std::string format = formatString(pMsg, std::forward<Args>(args)...);
  // queue=>Messages are placed in a queue and processed asynchronously. This method does not immediately record logs, but stores them in memory for later
  // processing. direct=>The log message is immediately written to the specified output (such as a file, console, etc.). This method is executed synchronously
  // when sending logs.
  if (connectionMode == Direct)
  {
    notify<level>(format);
  }
}

template <LogLevel level>
inline void Console::notify(const std::string& msg)
{
  sendLogToObserver(level, msg);
}

template <LogLevel level, typename... Args>
inline void Console::sendWithCtx(const char* file, int line, const char* func, const char* msg, Args&&... args)
{
  // use std::format(C++20) to format the string
  std::string format = std::format("{}", std::vformat(msg, std::make_format_args(args...)));
  std::string logMessage = formatLog(format, file, line, func, level);

  if (connectionMode == Direct)
  {
    notify<level>(logMessage);
  }
}

} // namespace log
} // namespace common

#endif // IVYCONSOLE_INL_