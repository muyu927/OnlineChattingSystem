#include "Awaiter_AsyncAccept.h"
#include "Scheduler_epoll.h"
#include <iostream>
#include <thread>

Awaiter_AsyncAccept::Awaiter_AsyncAccept(int lfd, Scheduler_epoll* scheduler)
	: m_lfd(lfd), m_scheduler(scheduler), m_is_ctl_lfd(false)
{
	std::cout << "初始化Awaiter_AsyncAccept等待体\n";
}

void Awaiter_AsyncAccept::await_suspend(std::coroutine_handle<> handle) noexcept
{
	if (m_scheduler != nullptr && !m_is_ctl_lfd)
	{
		m_scheduler->add_event(m_lfd, handle, EPOLLIN);
		std::cout << "服务器监听文件描述符为：" << m_lfd << '\n';
		std::cout << "监听协程句柄：" << handle.address() << std::endl;
		m_is_ctl_lfd = true;
	}
}

int Awaiter_AsyncAccept::await_resume() noexcept
{
	std::cout << "获取到新的连接请求！\n";
	m_client_address_len = sizeof(m_client_address);
	// 由于Scheduler调度，此时一定有新连接进来
	m_cfd = accept4(m_lfd, (sockaddr*)(&m_client_address), &m_client_address_len, SOCK_NONBLOCK);
	std::cout << "新连接的文件描述符:" << m_cfd << '\n';

	return m_cfd;
}
