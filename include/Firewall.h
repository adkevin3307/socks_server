#pragma once

#include <string>

#include "constant.h"

using namespace std;

class Firewall {
public:
    Firewall();
    ~Firewall();

    static bool check(string address, CONSTANT::SOCKS_TYPE type);
};