#include "Common_Logger.h"
#include <stdarg.h>


Common_Logger::Common_Logger()
	: m_is_running(false)
{

}

Common_Logger& Common_Logger::getLoggerInstance()
{
	static Common_Logger instance;
	return instance;
}

void Common_Logger::initLoggerSystem()
{

}

void Common_Logger::startLoggerSystem()
{
	m_is_running = true;
	//m_task = comainLoggerSystem();
}

void Common_Logger::shutdownLoggerSystem()
{
	m_is_running = false;
}

void Common_Logger::log(LogType type, uint64_t co_id, const char* fmt, ...)
{
	if (!m_is_running) return;

	// 格式化日志内容
	char buf[4096];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	// 构造日志消息体
	LogMsg msg;
	msg.type = type;
	msg.level = getLogTypeLevel(type);
	msg.timestamp = std::chrono::system_clock::now();
	msg.thread_id = gettid();
	msg.coroutine_id = co_id;
	msg.content = buf;

	// 放入消息队列
	m_log_queue.push(msg);
}

void Common_Logger::log(LogType type, const char* fmt, ...)
{
	if (!m_is_running) return;

	// 处理可变参数并转发调用
	va_list args;
	va_start(args, fmt);
	log(type, 0, fmt, args);
	va_end(args);
}

LogLevel Common_Logger::getLogTypeLevel(LogType type)
{
	LogLevel level = LogLevel::NORMAL;
	switch (type)
	{
	case LogType::LOG_DEBUG:
		level = LogLevel::DEBUG;
		break;
	case LogType::LOG_INFO:
		level = LogLevel::INFO;
		break;
	case LogType::LOG_NORMAL:
		level = LogLevel::NORMAL;
		break;
	case LogType::LOG_WARNING:
		level = LogLevel::WARNING;
		break;
	case LogType::LOG_ERROR:
		level = LogLevel::ERROR;
		break;
	case LogType::LOG_FATAL:
		level = LogLevel::FATAL;
		break;
	default:
		level = LogLevel::DEBUG;
		break;
	}

	return level;
}

void Common_Logger::comainLoggerSystem()
{
	//Awaiter_LoggerSystem a{};
	while (m_is_running)
	{
		//co_await a;
	}

	//co_return;
}