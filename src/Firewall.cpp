#include "Firewall.h"

#include <map>
#include <regex>
#include <vector>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <iostream>

using namespace std;

Firewall::Firewall()
{
}

Firewall::~Firewall()
{
}

bool Firewall::check(string address, CONSTANT::SOCKS_TYPE type)
{
    map<CONSTANT::SOCKS_TYPE, vector<string>> allowlist;
    boost::filesystem::path path = boost::filesystem::current_path() / "socks.conf";

    if (boost::filesystem::is_regular_file(path)) {
        boost::filesystem::fstream file;
        file.open(path);

        string _command, _type, _address;
        while (file >> _command >> _type >> _address) {
            if (_command != "permit") continue;

            CONSTANT::SOCKS_TYPE socks_type;
            switch (_type[0]) {
                case 'c':
                    socks_type = CONSTANT::SOCKS_TYPE::CONNECT;

                    break;
                case 'b':
                    socks_type = CONSTANT::SOCKS_TYPE::BIND;

                    break;
                default:
                    socks_type = CONSTANT::SOCKS_TYPE::NONE;

                    break;
            }

            boost::replace_all(_address, ".", "\\.");
            boost::replace_all(_address, "*", "[0-9]{1,3}");
            allowlist[socks_type].push_back(_address);
        }

        file.close();
    }

    if (allowlist.find(type) == allowlist.end()) return false;

    for (auto address_regex : allowlist[type]) {
        smatch string_match;

        if (regex_match(address, string_match, regex(address_regex))) return true;
    }

    return false;
}