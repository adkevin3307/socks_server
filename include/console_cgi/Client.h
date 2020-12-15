#pragma once

#include <map>
#include <vector>
#include <string>
#include <boost/asio.hpp>

#include "console_cgi/ClientSession.h"

using namespace std;

class Client {
private:
    vector<string> keys;
    map<string, string> information;
    shared_ptr<boost::asio::io_context> _io_context;
    vector<shared_ptr<ClientSession>> sessions;

public:
    Client(shared_ptr<boost::asio::io_context> io_context);
    Client(shared_ptr<boost::asio::io_context> io_context, string query);
    ~Client();

    string html_response();
    string html_template();
    void execute_sessions();
};