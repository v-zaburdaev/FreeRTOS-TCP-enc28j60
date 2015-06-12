#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>

extern const char *Request_type_str[];
struct Request_data;

class Request {
public:
    enum type {
        GET,
        HEAD,
        POST,
        PUT,
        DELETE,
        OPTIONS,
        TRACE,
        CONNECT,
        PATCH,
        WAT
    };
    static Request_data handle_request(const std::string &req_str);
};

struct Request_data {
    Request::type req_type;
    std::string url;
    std::string data;
};


#endif /* end of include guard: REQUEST_HPP */

