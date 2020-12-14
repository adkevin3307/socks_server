#include "Handler.h"

#include <iostream>

using namespace std;

Handler::Handler(shared_ptr<boost::asio::io_context> io_context, boost::asio::ip::tcp::socket socket, boost::asio::ip::tcp::endpoint endpoint)
    : _acceptor(*io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 0)),
      _server_socket(move(socket)),
      _client_socket(*io_context),
      _endpoint(endpoint)
{
    this->_buffer.resize(2);

    for (size_t i = 0; i < this->_buffer.size(); i++) {
        this->_buffer[i].resize(CONSTANT::MAX_BUFFER_SIZE);
    }
}

Handler::~Handler()
{
    for (size_t i = 0; i < this->_buffer.size(); i++) {
        this->_buffer[i].clear();
        this->_buffer.shrink_to_fit();
    }

    this->_buffer.clear();
    this->_buffer.shrink_to_fit();
}


void Handler::reply(uint8_t code, uint16_t port)
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

void Handler::pipe_socket(size_t id, boost::asio::ip::tcp::socket& read, boost::asio::ip::tcp::socket& write)
{
    auto self(shared_from_this());

    auto read_buffer = boost::asio::buffer(this->_buffer[id], this->_buffer[id].size());
    read.async_read_some(read_buffer, [=, &read, &write](boost::system::error_code error_code, size_t read_bytes) {
        if (!error_code) {
            auto write_buffer = boost::asio::buffer(this->_buffer[id], read_bytes);
            write.async_send(write_buffer, [=, &read, &write](boost::system::error_code error_code, size_t write_bytes) {
                if (!error_code) {
                    this->pipe_socket(id, read, write);
                }
                else {
                    cerr << "Session pipe write error: " << error_code.message() << '\n';
                }
            });
        }
        else if (error_code == boost::asio::error::eof) {
            read.close();
            write.close();
        }
        else {
            cerr << "Session pipe read error: " << error_code.message() << '\n';
        }
    });
}

void Handler::start(CONSTANT::SOCKS_TYPE type)
{
    auto self(shared_from_this());

    switch (type) {
        case CONSTANT::SOCKS_TYPE::CONNECT:
            this->_client_socket.async_connect(this->_endpoint, [this, self](const boost::system::error_code& error_code) {
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