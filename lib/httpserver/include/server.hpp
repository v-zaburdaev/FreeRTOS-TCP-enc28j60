#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <cstdio>
#include <stdint.h>

extern "C" {
#include "FreeRTOS.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_IP_Private.h"
#include "FreeRTOS_Sockets.h"
}

#include "debug.hpp"

#include "constants.hpp"

void HttpserverTask( void *pvParameters );

class Server {
public:
    static void create(uint16_t port);
    static void listen();
    static xSocket_t  accept();
    static std::string receive(xSocket_t clientid);
    static void send(xSocket_t clientid, const std::string &response);
    static void remove();
private:
    static xSocket_t socketfd;
    static char buffer[RECV_BUFF_SIZE];
};


#endif /* end of include guard: SERVER_HPP */
