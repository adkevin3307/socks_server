#include "console_cgi/Client.h"

#include <iostream>
#include <vector>
#include <boost/algorithm/string/trim.hpp>

#include "constant.h"
#include "console_cgi/QueryParser.h"

using namespace std;

Client::Client(shared_ptr<boost::asio::io_context> io_context)
{
    this->_io_context = shared_ptr<boost::asio::io_context>(io_context);

    this->keys = vector<string>{ "h", "p", "f" };
    this->information = QueryParser::parse(getenv("QUERY_STRING"));

    cout << this->html_response();
    fflush(stdout);

    cout << this->html_template();
    fflush(stdout);

    this->execute_sessions();
}

Client::Client(shared_ptr<boost::asio::io_context> io_context, string query)
{
    this->_io_context = shared_ptr<boost::asio::io_context>(io_context);

    this->keys = vector<string>{ "h", "p", "f" };
    this->information = QueryParser::parse(query);
}

Client::~Client()
{
}

string Client::html_response()
{
    string s = "Content-type: text/html\r\n\r\n";

    return s;
}

string Client::html_template()
{
    string s;
    int connections = 0;

    stringstream ss, result;
    ss << CONSTANT::CONSOLE_HTML;

    while (getline(ss, s)) {
        boost::trim(s);

        if (s == "TITLE") {
            result << "<tr>" << '\n';

            for (size_t i = 0; i < this->information.size() / this->keys.size(); i++) {
                string host = this->information[this->keys[0] + to_string(i)];
                string port = this->information[this->keys[1] + to_string(i)];

                if (host != "" && port != "") {
                    connections += 1;

                    result << "<th scope='col'>" << '\n';
                    result << host << ':' << port << '\n';
                    result << "</th>" << '\n';
                }
            }

            result << "</tr>" << '\n';
        }
        else if (s == "CONTENT") {
            result << "<tr>" << '\n';

            for (auto i = 0; i < connections; i++) {
                result << "<td><pre id='s" << i << "' class='mb-0'></pre></td>" << '\n';
            }

            result << "</tr>" << '\n';
        }
        else {
            result << s << '\n';
        }
    }

    return result.str();
}

void Client::execute_sessions()
{
    for (size_t i = 0; i < this->information.size() / this->keys.size(); i++) {
        string host = this->information[keys[0] + to_string(i)];
        string port = this->information[keys[1] + to_string(i)];
        string filename = this->information[keys[2] + to_string(i)];

        if (host != "" && port != "" && filename != "") {
            auto ptr = make_shared<ClientSession>(this->_io_context, i, host, port, filename);

            this->sessions.push_back(ptr);
            ptr->start();
        }
    }
}