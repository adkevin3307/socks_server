#pragma once

namespace CONSTANT {
    const static size_t MAX_BUFFER_SIZE = 4096;

    enum SOCKS_MODE {
        SOCKS4,
        SOCKS4A
    };

    enum SOCKS_TYPE {
        CONNECT,
        BIND,
        NONE
    };
};