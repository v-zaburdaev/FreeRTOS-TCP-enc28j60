#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <string>

enum { 
     RECV_BUFF_SIZE = 2048 
};

namespace srv_info {
    extern const char *http_version;
    extern const char *server_version;
}

#endif /* end of include guard: CONSTANTS_HPP */
