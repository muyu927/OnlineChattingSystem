#pragma once
#include <sys/epoll.h>
#include <coroutine>
#include <mutex>
#include <unordered_map>
#define EPOLL_WAIT_MAX_CONN 512

class Scheduler_epoll
{
public:
	Scheduler_epoll();
	~Scheduler_epoll();

	void add_event(int fd, std::coroutine_handle<> handle, uint32_t events);

	void remove_event(int fd);

	void run_accept_fd();

private:
	int m_epfd;
};
