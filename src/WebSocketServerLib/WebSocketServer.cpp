#include "WebSocketServer.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <unistd.h>
#include "Awaiter_AsyncAccept.h"
#include "Awaiter_HandleConnectionRecv.h"
#include "Scheduler_epoll.h"
#include "coroutine_task.h"
#include <format>



WebSocketServer::WebSocketServer(unsigned short port)
	: m_port(port), m_scheduler(new Scheduler_epoll)
{
	initServer();
}

WebSocketServer::~WebSocketServer()
{
	close(m_epfd);
	close(m_lfd);

	delete m_scheduler;
}

void WebSocketServer::initServer()
{
	// 1、初始化监听文件描述符
	initListenfd();

	// 2、开启监听协程
	task t = epollAccept();
	m_tasks.emplace_back(std::move(t));

	// 3、开启循环监听
	if (m_scheduler) m_scheduler->run_accept_fd();
}

void WebSocketServer::initListenfd()
{
	// 1、创建服务器监听文件描述符
	m_lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (m_lfd == -1)
	{
		perror("socket");
	}

	// 2、设置端口复用
	int opt = 1;
	if (-1 == setsockopt(m_lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
	{
		perror("setsockopt");
	}

	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_port = htons(m_port);
	address.sin_addr.s_addr = INADDR_ANY;

	// 3、绑定端口
	if (-1 == bind(m_lfd, (sockaddr*)(&address), sizeof(address)) )
	{
		perror("bind");
	}

	// 4、设置监听上限
	if (-1 == listen(m_lfd, LISTEN_MAX_CONN))
	{
		perror("listen");
	}
}

task WebSocketServer::epollAccept()
{
	Awaiter_AsyncAccept a{ m_lfd, m_scheduler };

	std::cout << "开启Awaiter_AsyncAccept等待体协程\n";
	while (true)
	{
		int cfd = co_await a;		
		if (cfd == -1)
		{
			continue;
		}

		task t = handleConnection(cfd);
		m_tasks.emplace_back(std::move(t));
	}

	co_return;
}

task WebSocketServer::handleConnection(int cfd)
{
	std::cout << "新连接：获取到新的客户端连接，cfd = " << cfd << '\n';

	char buf[1024];
	Awaiter_HandleConnectionRecv a{ cfd, m_scheduler, buf, sizeof(buf) };

	while (true)
	{
		ssize_t data_len = co_await a;

		if (data_len <= 0) break;

		std::string recevied_data(buf, data_len);
		std::cout << std::format("读取到客户端{}的数据:{}\n", cfd, recevied_data);
	}

	m_scheduler->remove_event(cfd);
	close(cfd);
	co_return;
}


