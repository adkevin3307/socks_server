#pragma once

#include <vector>
#include <boost/asio.hpp>

#include "Parser.h"
#include "Handler.h"

using namespace std;

class Session : public enable_shared_from_this<Session> {
private:
    shared_ptr<boost::asio::io_context> _io_context;
    boost::asio::ip::tcp::socket _socket;
    boost::asio::ip::tcp::resolver _resolver;
    vector<uint8_t> _buffer;
    Request request;
    vector<shared_ptr<Handler>> handlers;

    void do_read();
    void resolve_host();

public:
    Session(shared_ptr<boost::asio::io_context> io_context, boost::asio::ip::tcp::socket socket);
    ~Session();

    void start();
};
