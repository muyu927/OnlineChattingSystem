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

	// 1.��ʼ������������
	void initServer();

	// 2.��ʼ�������ļ�������
	void initListenfd();

	// 3.�������
	void epollAccept();

private:
	// �������˿�
	unsigned short m_port = 7999;
	// �����������ļ�������
	int m_lfd;

	int m_epfd;
	epoll_event m_ev;
	epoll_event m_evs[EPOLL_WAIT_MAX_CONN];
};