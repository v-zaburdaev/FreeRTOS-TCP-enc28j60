#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <unordered_map>
#include <cstdlib>
#include <stdint.h>

#include "debug.hpp"

#include "constants.hpp"
#include "html_file.hpp"
#include "request.hpp"



class Response {
public:
    enum type {
        OK,
        BAD_REQUEST,
        UNAUTHORIZED,
        NOT_FOUND,
        INTERNAL_SERVER_ERROR
    };

    static const char *type_str[];

    static std::string generate_response(const Html_file &file, Response::type res_type);
};

#endif /* end of include guard: RESPONSE_HPP */
