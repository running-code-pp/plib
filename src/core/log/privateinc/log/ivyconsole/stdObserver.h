//--------------------------------------------------------------------------------------------------
// Copyright (c) ZIXEL 2024
// History:
// 2024-07-12 IVY: V0.0
//--------------------------------------------------------------------------------------------------
#include "log/common.h"
#include "global.h"
#include "log/ivyconsole/observer.h"
#include "log/ivyconsole/ivylogConfig.h"
#include <fstream>

namespace common {
namespace log {

/** The CmdConsoleObserver class
 *  This class is used by the main modules to write Console messages and logs the system con.
 */
class StdObserver : public Observer
{
  public:
  StdObserver();
  virtual ~StdObserver() {}
  void sendLog(const std::string& sMessage, LogLevel level) override;
  const char* name() const override { return "Console"; }

  private:
  bool _useColor;

#if defined(COMMON_OS_WINDOWS)
  struct ColorAttribute
  {
    uint16_t winAttribute;
  };
#else
  struct ColorAttribute
  {
    const char* ansiColor;
  };
#endif

  static const ColorAttribute FATAL_COLOR;
  static const ColorAttribute ERROR_COLOR;
  static const ColorAttribute WARNING_COLOR;
  static const ColorAttribute INFO_COLOR;
  static const ColorAttribute DEBUG_COLOR;
  static const ColorAttribute TRACE_COLOR;
  static const ColorAttribute RESET_COLOR;

  bool checkColorSupport();
  void setConsoleColor(const ColorAttribute& attr);
};

} // namespace log
} // namespace common
