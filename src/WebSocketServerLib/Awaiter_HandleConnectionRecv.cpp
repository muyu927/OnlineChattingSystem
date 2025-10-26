#include "Awaiter_HandleConnectionRecv.h"
#include "Scheduler_epoll.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>

Awaiter_HandleConnectionRecv::Awaiter_HandleConnectionRecv(int cfd, Scheduler_epoll* scheduler, char* buf, size_t len)
	: m_cfd(cfd), m_scheduler(scheduler), m_buf(buf), m_buf_len(len), m_is_add_cfd(false)
{
}

void Awaiter_HandleConnectionRecv::await_suspend(std::coroutine_handle<> handle) noexcept
{
	if (m_scheduler != nullptr && !m_is_add_cfd) 
	{
		m_scheduler->add_event(m_cfd, handle, EPOLLIN);
		m_is_add_cfd = true;
	}
}

size_t Awaiter_HandleConnectionRecv::await_resume() noexcept
{
	m_bytes_read = recv(m_cfd, m_buf, m_buf_len, 0); // 非阻塞读
	return m_bytes_read;
}