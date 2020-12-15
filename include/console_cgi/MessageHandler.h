#pragma once

#include <string>
#include <boost/asio.hpp>

#include "constant.h"

using namespace std;

class MessageHandler {
private:
    static void html_escape(string& s);

public:
    MessageHandler();
    ~MessageHandler();

    static void output(string id, string s, CONSTANT::OUTPUT_TYPE type);
};