#include "console_cgi/ClientSession.h"

#include <fstream>
#include <boost/algorithm/string/trim.hpp>

#include "constant.h"
#include "console_cgi/MessageHandler.h"

using namespace std;

ClientSession::ClientSession(shared_ptr<boost::asio::io_context> io_context, int id, string host, string port, string filename)
    : _socket(*io_context), _resolver(*io_context)
{
    this->id = to_string(id);
    this->host = host;
    this->port = port;
    this->filename = filename;
    this->socks_host = "";
    this->socks_port = "";

    this->index = 0;
    this->_buffer.resize(CONSTANT::MAX_BUFFER_SIZE);

    fstream file;
    file.open("test_case/" + this->filename, ios::in);

    string line;
    while (getline(file, line)) {
        boost::trim(line);

        this->commands.push_back(line);
    }

    file.close();
}

ClientSession::~ClientSession()
{
}

void ClientSession::resolve_host()
{
    boost::asio::ip::tcp::resolver::query query(this->host, this->port);
    this->_resolver.async_resolve(query, [this](const boost::system::error_code& error_code, boost::asio::ip::tcp::resolver::iterator it) {
        if (!error_code) {
            this->connect_host(it);
        }
        else {
            string error_message = "Client resolve error: " + error_code.message() + '\n';
            MessageHandler::output(this->id, error_message, CONSTANT::OUTPUT_TYPE::STDERR);
        }
    });
}

void ClientSession::connect_host(boost::asio::ip::tcp::resolver::iterator it)
{
    if (this->socks_host.empty() || this->socks_port.empty()) {
        this->_socket.async_connect(it->endpoint(), [this](const boost::system::error_code& error_code) {
            if (!error_code) {
                this->do_read();
            }
            else {
                string error_message = "Client connect error: " + error_code.message() + '\n';
                MessageHandler::output(this->id, error_message, CONSTANT::OUTPUT_TYPE::STDERR);
            }
        });
    }
    else {
        boost::asio::ip::tcp::resolver::query query(this->socks_host, this->socks_port);
        this->_resolver.async_resolve(query, [this, it](const boost::system::error_code& error_code, boost::asio::ip::tcp::resolver::iterator socks_it) {
            if (!error_code) {
                this->_socket.async_connect(socks_it->endpoint(), [this, it](const boost::system::error_code& error_code) {
                    if (!error_code) {
                        vector<uint8_t> packet(9, 0x00);
                        packet[0] = 4;
                        packet[1] = 1;
                        packet[2] = it->endpoint().port() / 256;
                        packet[3] = it->endpoint().port() % 256;

                        uint32_t socks_address = it->endpoint().address().to_v4().to_uint();
                        for (auto i = 0; i < 4; i++) {
                            uint32_t mask = (i << ((4 - i - 1) * 8));

                            packet[i + 4] = socks_address / mask;
                            socks_address %= mask;
                        }

                        auto write_buffer = boost::asio::buffer(packet, 9);
                        boost::asio::async_write(this->_socket, write_buffer, [this](boost::system::error_code error_code, size_t bytes) {
                            if (!error_code) {
                                auto read_buffer = boost::asio::buffer(this->_buffer, this->_buffer.size());
                                this->_socket.async_read_some(read_buffer, [this](boost::system::error_code error_code, size_t bytes) {
                                    if (!error_code) {
                                        if (bytes == 8 && this->_buffer[1] == 0x5A) {
                                            this->do_read();
                                        }
                                    }
                                    else {
                                        string error_message = "Client write socks error: " + error_code.message() + '\n';
                                        MessageHandler::output(this->id, error_message, CONSTANT::OUTPUT_TYPE::STDERR);
                                    }
                                });
                            }
                            else {
                                string error_message = "Client write socks error: " + error_code.message() + '\n';
                                MessageHandler::output(this->id, error_message, CONSTANT::OUTPUT_TYPE::STDERR);
                            }
                        });
                    }
                    else {
                        string error_message = "Client connect socks error: " + error_code.message() + '\n';
                        MessageHandler::output(this->id, error_message, CONSTANT::OUTPUT_TYPE::STDERR);
                    }
                });
            }
            else {
                string error_message = "Client resolve socks error: " + error_code.message() + '\n';
                MessageHandler::output(this->id, error_message, CONSTANT::OUTPUT_TYPE::STDERR);
            }
        });
    }
}

void ClientSession::do_read()
{
    auto handle_buffer = boost::asio::buffer(this->_buffer, this->_buffer.size());
    this->_socket.async_read_some(handle_buffer, [this](const boost::system::error_code& error_code, size_t bytes) {
        if (!error_code) {
            string s = "";
            for (size_t i = 0; i < bytes; i++) {
                s += this->_buffer[i];
            }

            MessageHandler::output(this->id, s, CONSTANT::OUTPUT_TYPE::STDOUT);

            if (s.find('%') != string::npos) {
                this->do_write();
            }

            this->do_read();
        }
        else if (error_code != boost::asio::error::eof && error_code != boost::asio::error::operation_aborted) {
            string error_message = "Client read error: " + error_code.message() + '\n';
            MessageHandler::output(this->id, error_message, CONSTANT::OUTPUT_TYPE::STDERR);
        }
    });
}

void ClientSession::do_write()
{
    string command = this->commands[this->index++];

    if (command.empty()) {
        MessageHandler::output(this->id, "\n", CONSTANT::OUTPUT_TYPE::STDOUT);
    }
    else {
        command += '\n';

        auto handle_buffer = boost::asio::buffer(command, command.length());
        this->_socket.async_send(handle_buffer, [this, command](boost::system::error_code error_code, size_t bytes) {
            if (!error_code) {
                MessageHandler::output(this->id, command, CONSTANT::OUTPUT_TYPE::COMMAND);

                if (command == "exit\n") {
                    this->_socket.close();

                    boost::system::error_code trash;
                    this->_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, trash);
                }
                else {
                    this->do_read();
                }
            }
            else {
                string error_message = "Client write error: " + error_code.message() + '\n';
                MessageHandler::output(this->id, error_message, CONSTANT::OUTPUT_TYPE::STDERR);
            }
        });
    }
}

void ClientSession::start()
{
    this->resolve_host();
}

void ClientSession::start(string socks_host, string socks_port)
{
    this->socks_host = socks_host;
    this->socks_port = socks_port;
}