#pragma once
#include <coroutine>
#include <netinet/ip.h>

class Scheduler_epoll;

class Awaiter_AsyncAccept
{
public:
	Awaiter_AsyncAccept(int lfd, Scheduler_epoll* scheduler);

	bool await_ready() const noexcept { return false; }

	void await_suspend(std::coroutine_handle<> handle) noexcept;

	int await_resume() noexcept;

	
private:
	bool m_is_ctl_lfd;
	int m_lfd;
	Scheduler_epoll* m_scheduler;
	int m_cfd;
	sockaddr_in m_client_address;
	socklen_t m_client_address_len;
};