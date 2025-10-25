#include <cstdio>
#include <sys/socket.h>
#include "WebSocketServer.h"

int main()
{
    WebSocketServer server(7999);

    return 0;
}