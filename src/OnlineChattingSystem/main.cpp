#include <cstdio>
#include <sys/socket.h>
#include "WebSocketServer.h"

int main()
{
    std::cout << "开启服务器，监听端口：" << 7999 << '\n';
    WebSocketServer server(7999);

    return 0;
}