//--------------------------------------------------------------------------------------------------
// Copyright (c) ZIXEL 2024
// History:
// 2024-07-12 IVY: V0.0
//--------------------------------------------------------------------------------------------------
#include "log/common.h"
#include "log/ivyconsole/observer.h"
#include <fstream>

namespace common {
namespace log {
/** The LoggingConsoleObserver class
 *  This class is used by the main modules to write Console messages and logs to a file
 */
class FileObserver : public Observer
{
  public:
  explicit FileObserver(const char* sFileName);
  virtual ~FileObserver() { _fStream.close(); }
  bool isValid() const { return _isValid; }

  void sendLog(const std::string& iMessage, LogLevel iLevel) override;
  const char* name() const override { return "File"; }

  private:
  std::ofstream _fStream;
  bool _isValid = false;
};

} // namespace log
} // namespace common
