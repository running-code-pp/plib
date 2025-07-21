#pragma once

#include <string>
#include <memory>
#include "log/common.h"

namespace common {
namespace log {

/** The Observer Interface
 *  This class describes an Interface for logging . If you want to add a new
 *  "sink" to IVY's logging mechanism, then inherit this class. You'll also need to
 *  register your derived class with Console.
 *
 *  @see Observer
 */
class Observer
{
  public:
  Observer() : _bFatal(true), _bError(true), _bWarning(true), _bInfo(true), _bDebug(true), _bTrace(true) {}
  virtual ~Observer() {};

  /** send a log message at the given level.
   */
  virtual void sendLog(const std::string& iMsg, LogLevel level) = 0;

  /**
   * Returns whether the loglevel is active to output
   */
  inline bool isActive(LogLevel level) const
  {
    switch (level)
    {
      case LogLevel::FATAL: return _bFatal;
      case LogLevel::ERR: return _bError;
      case LogLevel::WARNING: return _bWarning;
      case LogLevel::INFO: return _bInfo;
      case LogLevel::DEBUG: return _bDebug;
      case LogLevel::TRACE: return _bTrace;
      default: return false;
    }
  }

  virtual const char* name() const { return nullptr; }

  // Getter and Setter for bFatal
  void setFatal(bool value) { _bFatal = value; }
  bool getFatal() const { return _bFatal; }

  // Getter and Setter for bError
  void setErr(bool value) { _bError = value; }
  bool getErr() const { return _bError; }

  // Getter and Setter for bWarning
  void setWrn(bool value) { _bWarning = value; }
  bool getWrn() const { return _bWarning; }

  // Getter and Setter for bInfo
  void setInfo(bool value) { _bInfo = value; }
  bool getInfo() const { return _bInfo; }

  // Getter and Setter for bDebug
  void setDebug(bool value) { _bDebug = value; }
  bool getDebug() const { return _bDebug; }

  // Getter and Setter for bTrace
  void setTrace(bool value) { _bTrace = value; }
  bool getTrace() const { return _bTrace; }

  // msg:Printing information in the console,
  // log:Record information in the log.
  private:
  bool _bFatal, _bError, _bWarning, _bInfo, _bDebug, _bTrace;
};

} // namespace log
} // namespace common