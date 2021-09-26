#include "httplib.h"
#include <iostream>

int main(void)
{
    httplib::Client cli("localhost", 58080);

    httplib::Params params_generate{
        {"mesh", "~/repos/cpp-service/test-server/input/teapot_10elev.stl"},
        {"config", "~/repos/cpp-service/test-server/input/config.json"},
        {"points", "~/repos/cpp-service/test-server/input/points.json"}
        };

    if (auto res = cli.Post("/generate", params_generate))
    {
        std::cout << res->status << std::endl;
        std::cout << res->get_header_value("Content-Type") << std::endl;
        std::cout << res->body << std::endl;
    }
    else
    {
        std::cout << "error code: " << res.error() << std::endl;
    }

    return 0;
}