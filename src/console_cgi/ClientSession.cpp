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