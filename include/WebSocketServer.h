#pragma once
#include <iostream>
#include <sys/epoll.h>

#define LISTEN_MAX_CONN 1024
#define EPOLL_WAIT_MAX_CONN 512

class WebSocketServer
{
public:
	WebSocketServer(unsigned short port);

	WebSocketServer() : WebSocketServer(7999){}

	~WebSocketServer();

	// 1.初始化服务器连接
	void initServer();

	// 2.初始化监听文件描述符
	void initListenfd();

	// 3.处理监听
	void epollAccept();

private:
	// 服务器端口
	unsigned short m_port = 7999;
	// 服务器监听文件描述符
	int m_lfd;

	int m_epfd;
	epoll_event m_ev;
	epoll_event m_evs[EPOLL_WAIT_MAX_CONN];
};