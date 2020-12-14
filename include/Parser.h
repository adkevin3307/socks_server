#pragma once

#include <string>
#include <vector>
#include <boost/optional.hpp>

#include "constant.h"

using namespace std;

class Request {
public:
    Request();
    ~Request();

    uint8_t version_number;
    CONSTANT::SOCKS_TYPE code;
    uint16_t port;
    uint32_t ip;
    string user_id, domain_name;
    CONSTANT::SOCKS_MODE mode;
};

class Parser {
public:
    Parser();
    ~Parser();

    static boost::optional<Request> parse(vector<uint8_t> buffer, size_t bytes);
};
