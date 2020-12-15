#include "console_cgi/MessageHandler.h"

#include <iostream>
#include <sstream>
#include <boost/algorithm/string/replace.hpp>

using namespace std;

MessageHandler::MessageHandler()
{
}

MessageHandler::~MessageHandler()
{
}

void MessageHandler::html_escape(string& s)
{
    boost::replace_all(s, "&",  "&amp;");
    boost::replace_all(s, "\"", "&quot;");
    boost::replace_all(s, "\'", "&apos;");
    boost::replace_all(s, "<",  "&lt;");
    boost::replace_all(s, ">",  "&gt;");
    boost::replace_all(s, "\n", "&NewLine;");
}

void MessageHandler::output(string id, string s, CONSTANT::OUTPUT_TYPE type)
{
    MessageHandler::html_escape(s);

    stringstream ss;
    switch (type) {
        case CONSTANT::OUTPUT_TYPE::STDOUT:
            ss << "<script>document.getElementById('s" << id << "').innerHTML += '" << s << "'</script>";

            break;
        case CONSTANT::OUTPUT_TYPE::STDERR:
            ss << "<script>document.getElementById('s" << id << "').innerHTML += '<red>" << s << "</red>'</script>";

            break;
        case CONSTANT::OUTPUT_TYPE::COMMAND:
            ss << "<script>document.getElementById('s" << id << "').innerHTML += '<green>" << s << "</green>'</script>";

            break;
        default:
            break;
    }

    string buffer = ss.str();

    cout << buffer;
    fflush(stdout);
}
