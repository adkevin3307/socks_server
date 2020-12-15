#include "console_cgi/QueryParser.h"

#include <cstdlib>
#include <algorithm>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

using namespace std;

QueryParser::QueryParser()
{
}

QueryParser::~QueryParser()
{
}

map<string, string> QueryParser::parse(string s)
{
    map<string, string> result;

    vector<string> tokens;
    split(tokens, s, boost::is_any_of("&"), boost::token_compress_on);

    for (auto token : tokens) {
        auto it = token.find_first_of('=');

        string key = token.substr(0, it);
        string value = (it == string::npos ? "" : token.substr(it + 1));

        result[key] = value;
    }

    return result;
}