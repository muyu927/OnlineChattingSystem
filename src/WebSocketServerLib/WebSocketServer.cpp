#include "WebSocketServer.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <unistd.h>



WebSocketServer::WebSocketServer(unsigned short port)
	: m_port(port)
{
	initServer();
}

WebSocketServer::~WebSocketServer()
{
	close(m_epfd);
	close(m_lfd);
}

void WebSocketServer::initServer()
{
	// 1����ʼ�������ļ�������
	initListenfd();

	// 2��epoll����
	epollAccept();
}

void WebSocketServer::initListenfd()
{
	// 1�����������������ļ�������
	m_lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (m_lfd == -1)
	{
		perror("socket");
	}

	// 2�����ö˿ڸ���
	int opt = 1;
	if (-1 == setsockopt(m_lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
	{
		perror("setsockopt");
	}

	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_port = htons(m_port);
	address.sin_addr.s_addr = INADDR_ANY;

	// 3���󶨶˿�
	if (-1 == bind(m_lfd, (sockaddr*)(&address), sizeof(address)) )
	{
		perror("bind");
	}

	// 4�����ü�������
	if (-1 == listen(m_lfd, LISTEN_MAX_CONN))
	{
		perror("listen");
	}
}

void WebSocketServer::epollAccept()
{
	m_epfd = epoll_create1(0);
	if (m_epfd == -1)
	{
		perror("epoll_create1");
	}

	m_ev.events = EPOLLIN | EPOLLET; // �ɶ��ұ�Ե����
	m_ev.data.fd = m_lfd;		// �����������ļ�������
	if (-1 == epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_lfd, &m_ev))
	{
		perror("epoll_ctl add lfd");
	}

	while (true)
	{
		int nfds = epoll_wait(m_epfd, m_evs, EPOLL_WAIT_MAX_CONN, -1);
		if (nfds == -1)
		{
			perror("epoll_wait");
		}

		for (int i = 0; i < nfds; ++i)
		{
			// ������������
			if (m_evs[i].data.fd == m_lfd)
			{

			}
			// �������¼�����
			else
			{

			}
		}
	}
}
