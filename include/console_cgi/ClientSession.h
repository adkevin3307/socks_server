#pragma once

#include <vector>
#include <string>
#include <boost/asio.hpp>

using namespace std;

class ClientSession : public enable_shared_from_this<ClientSession> {
private:
    int index;
    string id, host, port, filename;
    vector<string> commands;
    vector<char> _buffer;
    boost::asio::ip::tcp::socket _socket;
    boost::asio::ip::tcp::resolver _resolver;

    void resolve_host();
    void connect_host(boost::asio::ip::tcp::resolver::iterator it);
    void do_read();
    void do_write();

public:
    ClientSession(shared_ptr<boost::asio::io_context> io_context, int id, string host, string port, string filename);
    ~ClientSession();

    void start();
};