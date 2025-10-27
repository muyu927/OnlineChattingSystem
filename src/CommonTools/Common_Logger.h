#pragma once
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <fstream>

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

    // 1������Ԫ�أ�����������������
    bool push(const T& item);
    

    // 2��ȡ��Ԫ�أ�������ֱ����Ԫ�ػ�ֹͣ��
    bool pop(T& item, bool& stop);
    

    // 3����ն���
    void clear();

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
	void initLoggerSystem(std::string log_path, size_t max_file_size, int max_files);

	// 3��������־ϵͳ����
	void startLoggerSystem();

	// 4����ͣ��־ϵͳ����
	void shutdownLoggerSystem();


    // 6����־�ӿ�
    void log(LogType type, uint64_t co_id, const char* fmt, ...);

    // 7����Э��id��־�ӿ�
    void log(LogType type, const char* fmt, ...);

private:
    // 1����־ϵͳ������
    void loggerSystemFunc();

    // 2����ȡ��Ӧ��־���͵���־�ȼ�
    LogLevel getLogTypeLevel(LogType type);

    // 3��������������д���ļ�
    bool writeLogMsgToFile(std::vector<LogMsg>& log_datas);

    // 4���򿪵�ǰ��־�ļ�
    bool openCurrentFile();

    // 5��ʱ�����ʽ��
    std::string formatTime(std::chrono::system_clock::time_point& timestamp);

    // 6��������־���ͷ���������Ϣ
    std::string getLogTypeInfo(LogType type);

    // 7����־����
    void RotateLog();

    // 8������ʣ����־(�����˳�ʱ)
    void flushRemaining();

	// ���캯��˽�У����Ƴ���������Ϳ�����ֵ
	Common_Logger();
	Common_Logger(const Common_Logger&) = delete;
	Common_Logger& operator=(const Common_Logger&) = delete;

private:
    std::string m_log_path_;            // ��־�ļ�����·��
    std::ofstream m_current_file_;      // ��ǰ��־�ļ���
    size_t m_max_file_size_;            // �ļ����洢�ֽ�
    size_t m_current_file_size_;        // ��ǰ�ļ��Ĵ�С
    int m_max_files_;

	std::atomic<bool> m_is_running_;    // ���б�־
    SafeLogQueue<LogMsg> m_log_queue_;  // ��־����
    std::thread m_log_thread_;          // ��־�߳�

    std::mutex m_mtx_;                  // �ȴ���
    std::condition_variable m_cv_;       // �ȴ���������
};



template<typename T>
inline bool SafeLogQueue<T>::push(const T& item)
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

template<typename T>
inline bool SafeLogQueue<T>::pop(T& item, bool& stop)
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

template<typename T>
inline void SafeLogQueue<T>::clear()
{
    std::lock_guard<std::mutex> lock(mtx_);
    while (!queue_.empty())
    {
        queue_.pop();
    }
}