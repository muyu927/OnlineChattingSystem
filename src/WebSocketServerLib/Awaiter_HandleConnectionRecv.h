#pragma once
#include <unistd.h>
#include <coroutine>

class Scheduler_epoll;

class Awaiter_HandleConnectionRecv
{
public:
	Awaiter_HandleConnectionRecv(int cfd, Scheduler_epoll* scheduler, char* buf, size_t len);

	// 等待体对象调用时总是挂起
	bool await_ready() const noexcept  { return false; }

	void await_suspend(std::coroutine_handle<> handle) noexcept;

	size_t await_resume() noexcept;
private:
	bool m_is_add_cfd;
	int m_cfd;
	Scheduler_epoll* m_scheduler;
	char* m_buf;
	size_t m_buf_len;
	ssize_t m_bytes_read;
};
