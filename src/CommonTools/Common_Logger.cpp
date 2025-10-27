#include "Common_Logger.h"
#include <stdarg.h>
#include <thread>
#include <vector>
#include <format>
#include <ctime>
#include <memory>


Common_Logger::Common_Logger()
	: m_is_running_(false)
{

}

Common_Logger& Common_Logger::getLoggerInstance()
{
	static Common_Logger instance;
	return instance;
}

void Common_Logger::initLoggerSystem(std::string log_path, size_t max_file_size = 1024 * 1024 * 100, int max_files = 5)
{
	m_log_path_ = log_path;
	m_max_file_size_ = max_file_size;
	m_max_files_ = max_files;
}

void Common_Logger::startLoggerSystem()
{
	m_is_running_ = true;
	m_log_thread_ = std::thread(&Common_Logger::loggerSystemFunc, this);
}

void Common_Logger::shutdownLoggerSystem()
{
	m_is_running_ = false;
	m_cv_.notify_one();
	if (m_log_thread_.joinable())
	{
		m_log_thread_.join();
	}
	flushRemaining();
}

void Common_Logger::log(LogType type, uint64_t co_id, const char* fmt, ...)
{
	if (!m_is_running_) return;

	// ��ʽ����־����
	char buf[4096];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	// ������־��Ϣ��
	LogMsg msg;
	msg.type = type;
	msg.level = getLogTypeLevel(type);
	msg.timestamp = std::chrono::system_clock::now();
	msg.thread_id = gettid();
	msg.coroutine_id = co_id;
	msg.content = buf;

	// ������Ϣ����
	m_log_queue_.push(msg);
}

void Common_Logger::log(LogType type, const char* fmt, ...)
{
	if (!m_is_running_) return;

	// ����ɱ������ת������
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

bool Common_Logger::writeLogMsgToFile(std::vector<LogMsg>& log_datas)
{
	if (!openCurrentFile()) return false;

	std::string log_str;
	// [ʱ��] [��־����] [tid:�߳�id | cid:Э��id] ����
	for (LogMsg& msg : log_datas)
	{
		log_str += std::format(
			"[{}] [{}] [tid:{} | cid:{}] {}",
			formatTime(msg.timestamp),
			getLogTypeInfo(msg.type),
			msg.thread_id,
			msg.coroutine_id,
			msg.content
		);
		log_str += '\n';
	}
	m_current_file_.write(log_str.data(), log_str.size());
	m_current_file_.flush();	// ����д���ļ�

	m_current_file_size_ += log_str.size();
	if (m_current_file_size_ >= m_max_file_size_)
	{

	}
}

bool Common_Logger::openCurrentFile()
{
	if (m_current_file_.is_open()) return true;

	//auto now = std::chrono::system_clock::now();
	//auto now_time_t = std::chrono::system_clock::to_time_t(now);
	//std::tm now_tm;
	//localtime_r(&now_time_t, &now_tm); // �̰߳�ȫ
	//std::string datetime = std::format(
	//	"{:04d}{:02d}{:02d}_{:02d}{:02d}{:02d}", 
	//	now_tm.tm_year + 1900,  // tm_year�Ǵ�1900�꿪ʼ��ƫ��������+1900
	//	now_tm.tm_mon + 1,      // tm_mon��0-11����+1
	//	now_tm.tm_mday,         // �գ�1-31��
	//	now_tm.tm_hour,         // ʱ��0-23��
	//	now_tm.tm_min,          // �֣�0-59��
	//	now_tm.tm_sec           // �루0-59��
	//);
	//std::string filename = m_log_path_ + "/" + datetime + ".log";
	std::string filename = m_log_path_ + "/RunLog.log";
	m_current_file_.open(filename, std::ios::app | std::ios::binary);
	if (!m_current_file_.is_open())
	{
		perror("open file");
		return false;
	}

	m_current_file_.seekp(0, std::ios::end);
	m_current_file_size_ = m_current_file_.tellp();

	return true;
}

std::string Common_Logger::formatTime(std::chrono::system_clock::time_point& timestamp)
{
	std::string res;

	// 1����ʱ���ת��Ϊ�뼶���ȵ�time_t
	auto now_time_t = std::chrono::system_clock::to_time_t(timestamp);
	// 2��ת��Ϊ����ʱ���tm�ṹ�������ȡ������ʱ����
	std::tm now_tm;
	localtime_r(&now_time_t, &now_tm); // �̰߳�ȫ

	// 3����ȡ���벿��
	auto sec_point = std::chrono::system_clock::from_time_t(now_time_t);
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp.time_since_epoch() % std::chrono::seconds(1)).count();

	// 4��ʱ���ʽ���洢
	res = std::format(
		"{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}.{:03d}",
		now_tm.tm_year + 1900,  // tm_year�Ǵ�1900�꿪ʼ��ƫ��������+1900
		now_tm.tm_mon + 1,      // tm_mon��0-11����+1
		now_tm.tm_mday,         // �գ�1-31��
		now_tm.tm_hour,         // ʱ��0-23��
		now_tm.tm_min,          // �֣�0-59��
		now_tm.tm_sec,          // �루0-59��
		ms                      // ���루0-999��
	);

	return res;
}

std::string Common_Logger::getLogTypeInfo(LogType type)
{
	std::string res;
	switch (type)
	{
	case LogType::LOG_DEBUG:
		res = "DEBUG";
		break;
	case LogType::LOG_INFO:
		res = "INFO";
		break;
	case LogType::LOG_NORMAL:
		res = "NORMAL";
		break;
	case LogType::LOG_WARNING:
		res = "WARNING";
		break;
	case LogType::LOG_ERROR:
		res = "ERROR";
		break;
	case LogType::LOG_FATAL:
		res = "FATAL";
		break;
	default:
		res = "UNKNOWN";
		break;
	}

	return res;
}

void Common_Logger::RotateLog()
{
}

void Common_Logger::flushRemaining()
{
	std::vector<LogMsg> log_datas;
	LogMsg msg;
	bool stop = true;
	while (m_log_queue_.pop(msg, stop))
	{
		log_datas.emplace_back(std::move(msg));
	}
	if (!log_datas.empty())
	{
		writeLogMsgToFile(log_datas);
	}
	if (m_current_file_.is_open())
	{
		m_current_file_.close();
	}
}

void Common_Logger::loggerSystemFunc()
{
	while (m_is_running_)
	{
		std::vector<LogMsg> log_datas;
		LogMsg msg;
		bool stop = false;
		for (int i = 0; i < 100; ++i)
		{
			if (m_log_queue_.pop(msg, stop))
			{
				log_datas.emplace_back(msg);
				if (msg.level >= LogLevel::WARNING)
				{
					// ����д����־��Ϣ
				}				
			}
			else
			{
				break;
			}
		}

		if (!log_datas.empty())
		{
			writeLogMsgToFile(log_datas);
		}
		else
		{
			// ������ʱ�ȴ�(�������ѯ)
			std::unique_lock<std::mutex> lock(m_mtx_);
			m_cv_.wait_for(lock, std::chrono::milliseconds(100));
		}
	}

	//co_return;
}