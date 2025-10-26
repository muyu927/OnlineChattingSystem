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
    LogType type;                                       // 日志类型
    LogLevel level;                                     // 日志等级
    std::chrono::system_clock::time_point timestamp;    // 时间戳
    pid_t thread_id;                                    // 线程id
    uint64_t coroutine_id;                              // 协程id
    std::string content;                                    // 日志内容
};

// 线程安全队列（生产者-消费者模型）
template <typename T>
class SafeLogQueue 
{
public:
    SafeLogQueue(size_t max_size = 10000) : max_size_(max_size) {}

    // 放入元素（非阻塞，满则丢弃）
    bool push(const T& item) 
    {
        std::lock_guard<std::mutex> lock(mtx_);
        if (queue_.size() >= max_size_) 
        {
            return false; // 队列满，丢弃日志（避免阻塞业务）
        }
        queue_.push(item);
        cv_.notify_one(); // 通知消费者
        return true;
    }

    // 取出元素（阻塞，直到有元素或停止）
    bool pop(T& item, bool& stop) 
    {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this, &stop]() {
            return !queue_.empty() || stop;
            });
        if (stop) return false; // 退出信号
        item = queue_.front();
        queue_.pop();
        return true;
    }

    // 清空队列
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
    size_t max_size_; // 最大队列长度，防止内存爆炸
};

class Common_Logger
{
public:
	// 1、日志系统单例模式
	static Common_Logger& getLoggerInstance();

	// 2、初始化日志系统
	void initLoggerSystem();

	// 3、启动日志系统服务
	void startLoggerSystem();

	// 4、暂停日志系统服务
	void shutdownLoggerSystem();


    // 6、日志接口
    void log(LogType type, uint64_t co_id, const char* fmt, ...);

    // 7、无协程id日志接口
    void log(LogType type, const char* fmt, ...);

    // 8、获取对应日志类型的日志等级
    LogLevel getLogTypeLevel(LogType type);

private:
    // 1、日志系统协程
    void comainLoggerSystem();

	// 构造函数私有，且移除拷贝构造和拷贝赋值
	Common_Logger();
	Common_Logger(const Common_Logger&) = delete;
	Common_Logger& operator=(const Common_Logger&) = delete;

private:
	bool m_is_running;
	task m_task;
    SafeLogQueue<LogMsg> m_log_queue;
};

