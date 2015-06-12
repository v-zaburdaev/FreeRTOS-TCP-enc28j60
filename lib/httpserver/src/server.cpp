#include "server.hpp"
#include "request.hpp"
#include "response.hpp"
#include "file_resources.hpp"

xSocket_t Server::socketfd = 0;
char Server::buffer[RECV_BUFF_SIZE] = {0};

void Server::create(uint16_t port)
{
    socketfd = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP);
    if (reinterpret_cast<void *>(socketfd) == FREERTOS_INVALID_SOCKET) {
        debug("Socket creation failed.\n");
        while(1);
    }

    freertos_sockaddr host_addr = {0};
    host_addr.sin_port = FreeRTOS_htons(port);

    if (reinterpret_cast<void *>(FreeRTOS_bind(socketfd, reinterpret_cast<freertos_sockaddr*>(&host_addr), sizeof(host_addr))) == FREERTOS_INVALID_SOCKET) {
        debug("IP and port binding failed.\n");
        while(1);
    }

}

void Server::remove()
{
    FreeRTOS_closesocket(socketfd);
}

void Server::listen()
{
    if ( reinterpret_cast<void *>(FreeRTOS_listen(socketfd, 20)) == FREERTOS_INVALID_SOCKET ) {
        debug("Listening failed.\n");
        while(1);
    }
}

xSocket_t Server::accept()
{
    xSocket_t clientfd = FreeRTOS_accept(socketfd, NULL, NULL);
    if ( clientfd == FREERTOS_INVALID_SOCKET) {
        debug("FreeRTOS, accept: invalid socket\n");
        return NULL;
    } else if ( clientfd ==  NULL) {
        debug("FreeRTOS, accept: timeout occured\n");
        return NULL;
    }
    return clientfd;
}

std::string Server::receive(xSocket_t clientid)
{
    int len = FreeRTOS_recv(clientid, buffer, sizeof(buffer), 0);
    if (len == -pdFREERTOS_ERRNO_ENOMEM) {
        debug("Socket, recv: not enough memory.\n");
        return std::string();
    } else if (len == -pdFREERTOS_ERRNO_ENOTCONN) {
        debug("Socket, recv: socket is not connected.\n");
        return std::string();
    } else if (len == -pdFREERTOS_ERRNO_EINVAL) {
        debug("Socket, recv: invalid argument.\n");
        return std::string();
    } else if (len < 0) {
        debug("Socket, recv: length is negitve.\n");
        return std::string();
    } else {
        return std::string(buffer, len); 
    }
}

void Server::send(xSocket_t clientid, const std::string &response)
{
    FreeRTOS_send(clientid, response.c_str(), response.size(), 0);
}

void HttpserverTask( void *pvParameters )
{
    std::unordered_map<std::string, Html_file*> file_umap = populate_file_umap();

    Server::create(80);

    debug("Listening...\n");
    Server::listen();

    // main loop
    while (1) {
        debug("Wait for connetion...\n");

        xSocket_t clientid = Server::accept();
        if (clientid != NULL) {
            debug("Connetion established...\n");

            std::string request = Server::receive(clientid);

            if (!request.empty()) {
                debug("== Received message: ==\n");

                Request_data req_data = Request::handle_request(request);
                debug("== Parsed args: ==\n%d %s\n", req_data.req_type, req_data.url.c_str());


                if (file_umap.count(req_data.url) == 0) {
                    std::string response_str = Response::generate_response(*file_umap["/404.html"], Response::type::NOT_FOUND);
                    debug("<< Sending message 404: >>..");
                    Server::send(clientid, response_str);
                    debug("...Done\n");
                } else {
                    std::string response_str = Response::generate_response(*file_umap[req_data.url], Response::type::OK);
                    debug("<< Sending message: >>..");
                    Server::send(clientid, response_str);
                    debug("...Done\n");
                }
            }
            FreeRTOS_shutdown(clientid, FREERTOS_SHUT_RDWR);
            FreeRTOS_closesocket(clientid);
        }

    }
}

//TODO: Use hardware random generator
static UBaseType_t ulNextRand=1234;

UBaseType_t uxRand( void )
{
const uint32_t ulMultiplier = 0x015a4e35UL, ulIncrement = 1UL;

	/* Utility function to generate a pseudo random number. */

	ulNextRand = ( ulMultiplier * ulNextRand ) + ulIncrement;
	return( ( int ) ( ulNextRand >> 16UL ) & 0x7fffUL );
}

