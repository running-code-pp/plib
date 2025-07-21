#ifndef PLIB_CORE_LOG_LOGGER_HPP
#define PLIB_CORE_LOG_LOGGER_HPP
#include <spdlog/spdlog.h>
#include <string>
#include <memory>
#include "config.hpp"
namespace spdlog {
	class logger;
}
namespace detail {
	// Safe formatting function that handles all potential formatting errors
	template <typename... Args>
	std::string safeFormat(const char* fmt, Args&&... args)
	{
		if constexpr (sizeof...(args) == 0)
		{
			return fmt;
		}

		try
		{
			return std::vformat(fmt, std::make_format_args(args...));
		}
		catch (const std::format_error& e)
		{
			// Provide friendly error message
			std::string errorMsg = std::string(fmt) + " [Format error: " + e.what() + "]";

			// Optional: Try to include parameter values (limited by type conversions)
			try
			{
				std::ostringstream oss;
				oss << " [Args:";
				int dummy[] = { 0, ((oss << ' ' << args), 0)... };
				(void)dummy;
				oss << "]";
				errorMsg += oss.str();
			}
			catch (...)
			{
				errorMsg += " [Args cannot be displayed]";
			}

			return errorMsg;
		}
		catch (const std::exception& e)
		{
			return std::string(fmt) + " [Exception: " + e.what() + "]";
		}
		catch (...)
		{
			return std::string(fmt) + " [Unknown exception]";
		}
	}
} // namespace detail

namespace plib::core::log {
	class Logger
	{
	public:
		Logger();
		virtual ~Logger();

		void init(const LogConfig& config);
		void shutdown();
		void setLevel(LogLevel level);
		LogLevel getLevel() const;

		void log(LogLevel level, const char* file, int line, const char* func, const char* msg);

		template <typename... Args>
		void logWithFmt(LogLevel level, const char* file, int line, const char* func, const char* msg, Args&&... args)
		{
			if (!m_logger)
			{
				return;
			}
			std::string formattedMsg;
			if constexpr (sizeof...(args) > 0)
			{
				try
				{
					formattedMsg = std::vformat(msg, std::make_format_args(args...));
				}
				catch (const std::exception& e)
				{
					formattedMsg = std::string(msg) + " [format error: " + e.what() + "]";
				}
			}
			else
			{
				formattedMsg = msg;
			}

			log(level, file, line, func, formattedMsg.c_str());
			/*spdlog::source_loc loc{ file, line, func };

			switch (level)
			{
			case LogLevel::TRACE: m_logger->log(loc, spdlog::level::trace, msg, std::forward<Args>(args)...); break;
			case LogLevel::DEBUG: m_logger->log(loc, spdlog::level::debug, msg, std::forward<Args>(args)...); break;
			case LogLevel::INFO: m_logger->log(loc, spdlog::level::info, msg, std::forward<Args>(args)...); break;
			case LogLevel::WARNING: m_logger->log(loc, spdlog::level::warn, msg, std::forward<Args>(args)...); break;
			case LogLevel::ERR: m_logger->log(loc, spdlog::level::err, msg, std::forward<Args>(args)...); break;
			case LogLevel::FATAL: m_logger->log(loc, spdlog::level::critical, msg, std::forward<Args>(args)...); break;
			default: m_logger->log(loc, spdlog::level::info, msg, std::forward<Args>(args)...); break;
			}*/
		}

		void flush();

	private:
		std::string generateLogFileName(const std::string& basePath, const std::string& appName);
		// convert log level
		spdlog::level::level_enum convertToSpdLogLevel(LogLevel level) const;
		LogLevel convertFromSpdLogLevel(spdlog::level::level_enum level) const;
		bool supportsDirectFormatting() const { return true; }

		std::shared_ptr<spdlog::logger> m_logger; // spdlog instance
		LogLevel m_currentLevel;                  // output log level
		bool m_initialized;                       // true if initialized
	};
} // namespace plib::core::log

#endif // PLIB_CORE_LOG_LOGGER_HPP