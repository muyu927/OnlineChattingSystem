#include "Scheduler_epoll.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <iostream>

Scheduler_epoll::Scheduler_epoll()
{
	m_epfd = epoll_create1(0);
	if (m_epfd == -1)
	{
		perror("epoll_create1");
	}
}

Scheduler_epoll::~Scheduler_epoll()
{
	close(m_epfd);
}

void Scheduler_epoll::add_event(int fd, std::coroutine_handle<> handle, uint32_t events)
{
	epoll_event ev{};
	ev.events = events;
	//ev.data.fd = fd;
	ev.data.ptr = handle.address();
	if (-1 == epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &ev))
	{
		perror("epoll_ctl add fd");
	}
	std::cout << "文件描述符:" << fd << " 上树，句柄:" << ev.data.ptr << '\n';
}

void Scheduler_epoll::remove_event(int fd)
{
	if (-1 == epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, nullptr))
	{
		perror("epoll_ctl del fd");
	}
	std::cout << "文件描述符:" << fd << " 从树上移除\n";
}

void Scheduler_epoll::run_accept_fd()
{
	epoll_event events[EPOLL_WAIT_MAX_CONN];
	std::cout << "主循环开启，开始监听客户端请求！\n";
	while (true)
	{
		int nfds = epoll_wait(m_epfd, events, EPOLL_WAIT_MAX_CONN, -1);
		if (nfds == -1)
		{
			perror("epoll_wait");
			continue;
		}
		for (int i = 0; i < nfds; ++i)
		{
			// 上树的event已经绑定了对应的等待器对象，使用对象指针恢复协程时，会恢复对应等待器对象的await_resume函数
			auto handle = std::coroutine_handle<>::from_address(events[i].data.ptr);
			handle.resume();
		}
	}
}
