#include "Session.h"

#include <iostream>

#include "constant.h"

using namespace std;

Session::Session(shared_ptr<boost::asio::io_context> io_context, boost::asio::ip::tcp::socket socket)
    : _socket(move(socket)), _resolver(*io_context)
{
    this->_io_context = shared_ptr<boost::asio::io_context>(io_context);
    this->_buffer.resize(CONSTANT::MAX_BUFFER_SIZE);
}

Session::~Session()
{
    this->_buffer.clear();
    this->_buffer.shrink_to_fit();

    this->handlers.clear();
    this->handlers.shrink_to_fit();
}

void Session::do_read()
{
    auto self(shared_from_this());

    auto handle_buffer = boost::asio::buffer(this->_buffer, this->_buffer.size());
    this->_socket.async_read_some(handle_buffer, [this, self](boost::system::error_code error_code, size_t bytes) {
        if (!error_code) {
            auto request = Parser::parse(this->_buffer, bytes);
            if (request != boost::none) {
                this->request = move(*request);

                cout << "==============================" << '\n';
                cout << "<S_IP>: " << this->_socket.remote_endpoint().address().to_string() << '\n';
                cout << "<S_PORT>: " << this->_socket.remote_endpoint().port() << '\n';
                cout << "<D_IP>: " << boost::asio::ip::address_v4(this->request.ip).to_string() << '\n';
                cout << "<D_PORT>: " << this->request.port << '\n';
                cout << "<Command>: " << (this->request.code == CONSTANT::SOCKS_TYPE::CONNECT ? "Connect" : "Bind") << '\n';
                cout << "<Reply>: Accept" << '\n';
                cout << "==============================" << '\n';

                this->resolve_host();
            }
        }
        else {
            cerr << "Session read error: " << error_code.message() << '\n';
        }
    });
}

void Session::resolve_host()
{
    switch (this->request.mode) {
        case CONSTANT::SOCKS_MODE::SOCKS4: {
            boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address_v4(this->request.ip), this->request.port);

            auto ptr = make_shared<Handler>(this->_io_context, move(this->_socket), move(endpoint));

            this->handlers.push_back(ptr);
            ptr->start(this->request.code);

            break;
        }
        case CONSTANT::SOCKS_MODE::SOCKS4A: {
            auto self(shared_from_this());

            boost::asio::ip::tcp::resolver::query query(this->request.domain_name);
            this->_resolver.async_resolve(query, [this, self](const boost::system::error_code& error_code, boost::asio::ip::tcp::resolver::iterator it) {
                if (!error_code) {
                    auto endpoint = it->endpoint();
                    endpoint.port(this->request.port);

                    auto ptr = make_shared<Handler>(this->_io_context, move(this->_socket), move(endpoint));

                    this->handlers.push_back(ptr);
                    ptr->start(this->request.code);
                }
                else {
                    cerr << "Session resolve error: " << error_code.message() << '\n';
                }
            });

            break;
        }
        default:
            break;
    }
}

void Session::start()
{
    this->do_read();
}
