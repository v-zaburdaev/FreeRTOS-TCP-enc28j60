#include "response.hpp"


const char *Response::type_str[] = {
    "200 OK\n",
    "400 Bad Request\n",
    "401 Unauthorized\n",
    "404 Not Found\n",
    "500 Internal Server Error\n"
};
/* example response

const char *indexhtml = 
    "HTTP/1.1 200 OK\n"
    "Date: Mon, 23 May 2005 22:38:34 GMT\n"
    "Server: httpserver/0.1.0 (Unix) (Arch linux)\n"
    "Last-Modified: Wed, 08 Jan 2003 23:11:55 GMT\n"
    "ETag: \"3f80f-1b6-3e1cb03b\"\n"
    "Content-Type: text/html; charset=UTF-8\n"
    "Content-Length: 131\n"
    "Accept-Ranges: bytes\n"
    "Connection: close\n"
    "\n"
    "<html>\n"
    "<head>\n"
    "  <title>An Example Page</title>\n"
    "</head>\n"
    "<body>\n"
    "  Hello World, this is a very simple HTML document.\n"
    "</body>\n"
    "</html>\n";
*/

std::string Response::generate_response(const Html_file &file, Response::type res_type)
{
    std::string response_str;
    response_str.reserve(4000);
    debug("Generating message...");
    response_str += srv_info::http_version;
    response_str += Response::type_str[res_type];
    response_str += srv_info::server_version;
    response_str += "Last-Modified: " ;
    response_str += std::string(file.last_modified);
    response_str += "\n";
    // TODO: response_str += ETag;
    response_str += "Content-Type: text/html; charset=UTF-8\n";
    response_str += "Content-Length: ";
    response_str +=std::string(file.filesize);
    response_str += "Accept-Ranges: bytes\n";
    response_str += "Connection: close\n\n";
    response_str += file.content;
    debug("..Done str_ptr: %p\n", &response_str);
 
    return response_str;
}



