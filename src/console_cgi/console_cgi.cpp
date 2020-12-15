#include <iostream>
#include <exception>
#include <boost/asio.hpp>

#include "console_cgi/Client.h"

using namespace std;

int main()
{
    try {
        shared_ptr<boost::asio::io_context> io_context(new boost::asio::io_context);

        Client client(io_context);

        io_context->run();
    }
    catch (exception& error) {
        cerr << "Console cgi exception: " << error.what() << '\n';
    }

    return 0;
}
