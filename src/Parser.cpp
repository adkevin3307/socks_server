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

boost::optional<Request> Parser::parse(string s)
{
    Request request;

    if (s.length() < 9) return boost::none;

    request.version_number = s[0];
    if (request.version_number != 4) return boost::none;

    if (s[1] == 1) {
        request.code = CONSTANT::SOCKS_TYPE::CONNECT;
    }
    else if (s[1] == 2) {
        request.code = CONSTANT::SOCKS_TYPE::BIND;
    }
    else {
        request.code = CONSTANT::SOCKS_TYPE::NONE;

        return boost::none;
    }

    size_t index;
    for (index = 2; index < 4; index++) {
        request.port = ((request.port << 8) | s[index]);
    }

    for (index = 4; index < 8; index++) {
        request.ip = ((request.ip << 8) | s[index]);
    }

    for (index = 8; index < s.length() && s[index] != 0; index++) {
        request.user_id += s[index];
    }

    if (request.ip > 0 && request.ip <= numeric_limits<uint8_t>::max() && index + 1 < s.length()) {
        request.mode = CONSTANT::SOCKS_MODE::SOCKS4A;

        request.domain_name = s.substr(index + 1, s.length() - 1);
    }

    return request;
}
