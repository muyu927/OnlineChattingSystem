#pragma once
#include "coroutine_task.h"
#include "Awaiter_LoggerSystem.h"
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <queue>

#define g_pLogger Common_Logger::getLoggerInstance()

enum class LogLevel
{
	DEBUG = 0,
	INFO = 1,
    NORMAL = 2,
	WARNING = 3,
	ERROR = 4,
	FATAL = 5
};

enum class LogType
{
	LOG_DEBUG = 0,
	LOG_INFO = 1,
    LOG_NORMAL = 2,
	LOG_WARNING = 3,
	LOG_ERROR = 4,
	LOG_FATAL = 5
};

struct LogMsg
{
    LogType type;                                       // ��־����
    LogLevel level;                                     // ��־�ȼ�
    std::chrono::system_clock::time_point timestamp;    // ʱ���
    pid_t thread_id;                                    // �߳�id
    uint64_t coroutine_id;                              // Э��id
    std::string content;                                    // ��־����
};

// �̰߳�ȫ���У�������-������ģ�ͣ�
template <typename T>
class SafeLogQueue 
{
public:
    SafeLogQueue(size_t max_size = 10000) : max_size_(max_size) {}

    // ����Ԫ�أ�����������������
    bool push(const T& item) 
    {
        std::lock_guard<std::mutex> lock(mtx_);
        if (queue_.size() >= max_size_) 
        {
            return false; // ��������������־����������ҵ��
        }
        queue_.push(item);
        cv_.notify_one(); // ֪ͨ������
        return true;
    }

    // ȡ��Ԫ�أ�������ֱ����Ԫ�ػ�ֹͣ��
    bool pop(T& item, bool& stop) 
    {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this, &stop]() {
            return !queue_.empty() || stop;
            });
        if (stop) return false; // �˳��ź�
        item = queue_.front();
        queue_.pop();
        return true;
    }

    // ��ն���
    void clear() 
    {
        std::lock_guard<std::mutex> lock(mtx_);
        while (!queue_.empty()) {
            queue_.pop();
        }
    }

private:
    std::queue<T> queue_;
    std::mutex mtx_;
    std::condition_variable cv_;
    size_t max_size_; // �����г��ȣ���ֹ�ڴ汬ը
};

class Common_Logger
{
public:
	// 1����־ϵͳ����ģʽ
	static Common_Logger& getLoggerInstance();

	// 2����ʼ����־ϵͳ
	void initLoggerSystem();

	// 3��������־ϵͳ����
	void startLoggerSystem();

	// 4����ͣ��־ϵͳ����
	void shutdownLoggerSystem();


    // 6����־�ӿ�
    void log(LogType type, uint64_t co_id, const char* fmt, ...);

    // 7����Э��id��־�ӿ�
    void log(LogType type, const char* fmt, ...);

    // 8����ȡ��Ӧ��־���͵���־�ȼ�
    LogLevel getLogTypeLevel(LogType type);

private:
    // 1����־ϵͳЭ��
    void comainLoggerSystem();

	// ���캯��˽�У����Ƴ���������Ϳ�����ֵ
	Common_Logger();
	Common_Logger(const Common_Logger&) = delete;
	Common_Logger& operator=(const Common_Logger&) = delete;

private:
	bool m_is_running;
	task m_task;
    SafeLogQueue<LogMsg> m_log_queue;
};

