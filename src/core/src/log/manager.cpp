#include <memory>
#include <string>
#include <iostream>
#include "log/manager.hpp"
#include "log/logger.hpp"

namespace plib::core::log {
	std::shared_ptr<Logger> LogManager::s_logger;

	void LogManager::init() { init(LogConfig()); }

	void LogManager::init(LogConfig logConfig)
	{
		if (s_logger) return;

		s_logger = std::make_shared<Logger>();
		s_logger->init(logConfig);
	}

	void LogManager::shutdown()
	{
		if (!s_logger) return;
		s_logger->shutdown();
		s_logger.reset();
	}

	std::shared_ptr<Logger>& LogManager::getLogger()
	{
		if (!s_logger)
		{
			init();
		}
		return s_logger;
	}

	void LogManager::setLevel(LogLevel level)
	{
		if (!s_logger)
		{
			init();
		}
		s_logger->setLevel(level);
	}

	void LogManager::flush()
	{
		if (s_logger)
		{
			s_logger->flush();
		}
	}
} //plib::core::log