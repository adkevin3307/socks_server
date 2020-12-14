#pragma once

#include <vector>
#include <boost/asio.hpp>

#include "constant.h"

using namespace std;

class Handler : public enable_shared_from_this<Handler> {
private:
    boost::asio::ip::tcp::acceptor _acceptor;
    boost::asio::ip::tcp::socket _server_socket, _client_socket;
    boost::asio::ip::tcp::endpoint _endpoint;
    vector<vector<uint8_t>> _buffer;

    void reply(uint8_t code, uint16_t port = 0);
    void pipe_socket(size_t id, boost::asio::ip::tcp::socket& read, boost::asio::ip::tcp::socket& write);

public:
    Handler(shared_ptr<boost::asio::io_context> io_context, boost::asio::ip::tcp::socket socket, boost::asio::ip::tcp::endpoint endpoint);
    ~Handler();

    void start(CONSTANT::SOCKS_TYPE type);
};