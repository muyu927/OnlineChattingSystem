#include <cstdio>
#include <sys/socket.h>
#include "WebSocketServer.h"

int main()
{
    WebSocketServer server(7999);
    std::cout << "开启服务器，监听端口：" << 7999 << '\n';

    return 0;
}