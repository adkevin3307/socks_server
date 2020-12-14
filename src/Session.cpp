#include "Session.h"

#include <iostream>

#include "constant.h"

using namespace std;

Session::Session(shared_ptr<boost::asio::io_context> io_context, boost::asio::ip::tcp::socket socket)
    : _server_socket(move(socket)), _client_socket(*io_context), _resolver(*io_context)
{
    this->_io_context = shared_ptr<boost::asio::io_context>(io_context);
    this->_buffer.resize(CONSTANT::MAX_BUFFER_SIZE);

    this->pipe_buffer.resize(2);
    this->pipe_buffer[0].resize(CONSTANT::MAX_BUFFER_SIZE);
    this->pipe_buffer[1].resize(CONSTANT::MAX_BUFFER_SIZE);
}

Session::~Session()
{
    this->_buffer.clear();
    this->_buffer.shrink_to_fit();

    this->pipe_buffer[0].clear();
    this->pipe_buffer[0].shrink_to_fit();
    this->pipe_buffer[1].clear();
    this->pipe_buffer[1].shrink_to_fit();

    this->pipe_buffer.clear();
    this->pipe_buffer.shrink_to_fit();
}

void Session::reply(uint8_t code)
{
    auto self(shared_from_this());

    vector<char> packet(8, 0x00);
    packet[1] = code;

    auto handle_buffer = boost::asio::buffer(packet, 8);
    boost::asio::async_write(this->_server_socket, handle_buffer, [this, self](boost::system::error_code error_code, size_t bytes) {
        if (error_code) {
            cerr << "Session reply error: " << error_code.message() << '\n';
        }
    });
}

void Session::do_read()
{
    auto self(shared_from_this());

    auto handle_buffer = boost::asio::buffer(this->_buffer, this->_buffer.size());
    this->_server_socket.async_read_some(handle_buffer, [this, self](boost::system::error_code error_code, size_t bytes) {
        if (!error_code) {
            auto request = Parser::parse(this->_buffer, bytes);
            if (request != boost::none) {
                this->request = move(*request);

                cout << "==============================" << '\n';
                cout << "<S_IP>: " << this->_server_socket.remote_endpoint().address().to_string() << '\n';
                cout << "<S_PORT>: " << this->_server_socket.remote_endpoint().port() << '\n';
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
    auto self(shared_from_this());

    switch (this->request.mode) {
        case CONSTANT::SOCKS_MODE::SOCKS4: {
            boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address_v4(this->request.ip), this->request.port);
            this->handle(move(endpoint));

            break;
        }
        case CONSTANT::SOCKS_MODE::SOCKS4A: {
            boost::asio::ip::tcp::resolver::query query(this->request.domain_name);
            this->_resolver.async_resolve(query, [this, self](const boost::system::error_code& error_code, boost::asio::ip::tcp::resolver::iterator it) {
                if (!error_code) {
                    auto endpoint = it->endpoint();
                    endpoint.port(this->request.port);

                    this->handle(endpoint);
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

void Session::handle(boost::asio::ip::tcp::endpoint endpoint)
{
    auto self(shared_from_this());

    switch (this->request.code) {
        case CONSTANT::SOCKS_TYPE::CONNECT:
            this->_client_socket.async_connect(endpoint, [this, self](const boost::system::error_code& error_code) {
                if (!error_code) {
                    this->reply(0x5A);

                    this->pipe_socket(0, this->_server_socket, this->_client_socket);
                    this->pipe_socket(1, this->_client_socket, this->_server_socket);
                }
                else {
                    cerr << "Session connect error: " << error_code.message() << '\n';
                }
            });

            break;
        case CONSTANT::SOCKS_TYPE::BIND:
            break;
        default:
            break;
    }
}

void Session::pipe_socket(size_t id, boost::asio::ip::tcp::socket& read, boost::asio::ip::tcp::socket& write)
{
    auto self(shared_from_this());

    auto read_buffer = boost::asio::buffer(this->pipe_buffer[id], this->pipe_buffer[id].size());
    read.async_read_some(read_buffer, [=, &read, &write](boost::system::error_code error_code, size_t read_bytes) {
        if (!error_code) {
            auto write_buffer = boost::asio::buffer(this->pipe_buffer[id], read_bytes);
            write.async_send(write_buffer, [=, &read, &write](boost::system::error_code error_code, size_t write_bytes) {
                if (!error_code) {
                    this->pipe_socket(id, read, write);
                }
                else {
                    cerr << "Session pipe write error: " << error_code.message() << '\n';
                }
            });
        }
        else {
            cerr << "Session pipe read error: " << error_code.message() << '\n';
        }
    });
}

void Session::start()
{
    this->do_read();
}
