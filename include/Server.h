#pragma once

#include <vector>
#include <boost/asio.hpp>

#include "Session.h"

using namespace std;

class Server {
private:
    shared_ptr<boost::asio::io_context> _io_context;
    boost::asio::ip::tcp::acceptor _acceptor;
    boost::asio::ip::tcp::socket _socket;
    vector<shared_ptr<Session>> sessions;

    void do_accept();

public:
    Server(shared_ptr<boost::asio::io_context> io_context, int port);
    ~Server();
};
