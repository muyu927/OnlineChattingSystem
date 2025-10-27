#pragma once
#include <iostream>
#include <sys/epoll.h>
#include <vector>
#include "coroutine_task.h"
#include "global_export.h"

#define LISTEN_MAX_CONN 1024
#define EPOLL_WAIT_MAX_CONN 512

class Scheduler_epoll;


class WEBSOCKET_SERVER_LIB_EXPORT WebSocketServer
{
public:
	WebSocketServer(unsigned short port);

	WebSocketServer() : WebSocketServer(7999){}

	~WebSocketServer();

	// 1.初始化服务器连接
	void initServer();

	// 2.初始化监听文件描述符
	void initListenfd();

	// 3.协程：用于异步读取连接状态
	task epollAccept();

	// 4.协程：用于处理连接
	task handleConnection(int cfd);

private:
	// 服务器端口
	unsigned short m_port = 7999;
	// 服务器监听文件描述符
	int m_lfd;

	int m_epfd;
	epoll_event m_ev;
	epoll_event m_evs[EPOLL_WAIT_MAX_CONN];

	// epoll协程调度器
	Scheduler_epoll* m_scheduler;
	task m_task;
	std::vector<task> m_tasks;	// 完成的协程要删除
	std::mutex m_tasks_mtx;		// 协程数组锁
};