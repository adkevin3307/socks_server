#pragma once

#include <map>
#include <string>

using namespace std;

class QueryParser {
public:
    QueryParser();
    ~QueryParser();

    static map<string, string> parse(string s);
};