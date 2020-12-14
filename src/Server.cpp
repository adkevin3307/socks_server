#include "Server.h"

#include <iostream>

using namespace std;

Server::Server(shared_ptr<boost::asio::io_context> io_context, int port)
    : _acceptor(*io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
      _socket(*io_context)
{
    this->_io_context = shared_ptr<boost::asio::io_context>(io_context);

    this->do_accept();
}

Server::~Server()
{
    this->sessions.clear();
    this->sessions.shrink_to_fit();
}

void Server::do_accept()
{
    this->_acceptor.async_accept(this->_socket, [this](boost::system::error_code error_code) {
        if (!this->_acceptor.is_open()) return;

        if (!error_code) {
            auto ptr = make_shared<Session>(this->_io_context, move(this->_socket));

            this->sessions.push_back(ptr);
            ptr->start();
        }
        else {
            cerr << "Server accept error: " << error_code.message() << '\n';
        }

        this->do_accept();
    });
}