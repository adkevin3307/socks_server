#pragma once

#include <vector>
#include <boost/asio.hpp>

#include "Parser.h"

using namespace std;

class Session : public enable_shared_from_this<Session> {
private:
    shared_ptr<boost::asio::io_context> _io_context;
    boost::asio::ip::tcp::socket _server_socket, _client_socket;
    boost::asio::ip::tcp::resolver _resolver;
    vector<uint8_t> _buffer;
    vector<vector<uint8_t>> pipe_buffer;
    Request request;

    void reply(uint8_t code);
    void do_read();
    void resolve_host();
    void handle(boost::asio::ip::tcp::endpoint endpoint);
    void pipe_socket(size_t id, boost::asio::ip::tcp::socket& read, boost::asio::ip::tcp::socket& write);

public:
    Session(shared_ptr<boost::asio::io_context> io_context, boost::asio::ip::tcp::socket socket);
    ~Session();

    void start();
};
