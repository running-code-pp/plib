#ifndef PLIB_CORE_LOG_MANAGER_HPP
#define PLIB_CORE_LOG_MANAGER_HPP

#include <memory>
#include <string>
#include "log/config.hpp"

namespace plib::core::log {
	class Logger;
	// manager of log system
	class  LogManager
	{
	public:
		// Initialize the log system
		static void init();
		static void init(LogConfig logConfig);

		// close the log system
		static void shutdown();

		// get the logger instance
		static std::shared_ptr<Logger>& getLogger();

		// set log level
		static void setLevel(LogLevel level);

		// flush the logs to ensure all logs are written immediately
		static void flush();

	private:

		// logger instance
		static std::shared_ptr<Logger> s_logger;
	};
} // namespace plib::core::log

#endif // PLIB_CORE_LOG_MANAGER_HPP