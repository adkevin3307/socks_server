#pragma once

#include <string>

using namespace std;

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

    enum OUTPUT_TYPE {
        STDOUT,
        STDERR,
        COMMAND,
    };

    const static string CONSOLE_HTML = R"(
        <!DOCTYPE html>
        <html lang='en'>

        <head>
            <meta charset='UTF-8'>
            <title>NP Project 3 Console</title>
            <link rel='stylesheet' href='https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/css/bootstrap.min.css' integrity='sha384-TX8t27EcRE3e/ihU7zmQxVncDAy5uIKz4rEkgIXeMed4M0jlfIDPvg6uqKI2xXr2' crossorigin='anonymous'>
            <link href='https://fonts.googleapis.com/css?family=Source+Code+Pro' rel='stylesheet'>
            <link rel='icon' type='image/png' href='https://cdn0.iconfinder.com/data/icons/small-n-flat/24/678068-terminal-512.png'>
            <style>
                * {
                    font-family: 'Source Code Pro', monospace;
                    font-size: 1rem !important;
                }

                body {
                    background-color: #212529;
                }

                pre {
                    color: #cccccc;
                }

                green {
                    color: #01b468;
                }

                red {
                    color: red;
                }
            </style>
        </head>

        <body>
            <table class='table table-dark table-bordered'>
                <thead>
                    TITLE
                </thead>
                <tbody>
                    CONTENT
                </tbody>
            </table>
        </body>

        </html>
    )";
};