#include "Parser.h"

#include <limits>

using namespace std;

Request::Request()
{
    this->version_number = 0;
    this->code = CONSTANT::SOCKS_TYPE::NONE;
    this->port = 0;
    this->ip = 0;
    this->user_id = "";
    this->domain_name = "";
    this->mode = CONSTANT::SOCKS_MODE::SOCKS4;
}

Request::~Request()
{
}

Parser::Parser()
{
}

Parser::~Parser()
{
}

boost::optional<Request> Parser::parse(vector<uint8_t> buffer, size_t bytes)
{
    Request request;

    if (bytes < 9) return boost::none;

    request.version_number = buffer[0];
    if (request.version_number != 4) return boost::none;

    if (buffer[1] == 1) {
        request.code = CONSTANT::SOCKS_TYPE::CONNECT;
    }
    else if (buffer[1] == 2) {
        request.code = CONSTANT::SOCKS_TYPE::BIND;
    }
    else {
        request.code = CONSTANT::SOCKS_TYPE::NONE;

        return boost::none;
    }

    size_t index;
    for (index = 2; index < 4; index++) {
        request.port = ((request.port << 8) | buffer[index]);
    }

    for (index = 4; index < 8; index++) {
        request.ip = ((request.ip << 8) | buffer[index]);
    }

    for (index = 8; index < bytes && buffer[index] != 0; index++) {
        request.user_id += (char)buffer[index];
    }

    if (request.ip > 0 && request.ip <= numeric_limits<uint8_t>::max() && index + 1 < bytes) {
        request.mode = CONSTANT::SOCKS_MODE::SOCKS4A;

        for (index = 9; index < bytes && buffer[index] != 0; index++) {
            request.domain_name += (char)buffer[index];
        }
    }

    return request;
}
