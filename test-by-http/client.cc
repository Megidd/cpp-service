#include "httplib.h"
#include <iostream>

int main(void)
{
    httplib::Client cli("127.0.0.1", 58080);

    httplib::Params params_getPoints{
        {"mesh", "~/repos/cpp-service/test-input/teapot_10elev.stl"},
        {"config", "~/repos/cpp-service/test-input/config.json"},
        {"slices", "~/repos/cpp-service/test-input/slices.json"},
        {"args", "~/repos/cpp-service/test-input/autoargs.json"},
        {"output", "~/auto_points.json"}
        };

    if (auto res = cli.Post("/get-points", params_getPoints))
    {
        std::cout << res->status << std::endl;
        std::cout << res->get_header_value("Content-Type") << std::endl;
        std::cout << res->body << std::endl;
    }
    else
    {
        std::cout << "get points: error code: " << res.error() << std::endl;
    }

    httplib::Params params_generate{
        {"mesh", "~/repos/cpp-service/test-input/teapot_10elev.stl"},
        {"config", "~/repos/cpp-service/test-input/config.json"},
        {"points", "~/repos/cpp-service/test-input/points.json"}};

    if (auto res = cli.Post("/generate", params_generate))
    {
        std::cout << res->status << std::endl;
        std::cout << res->get_header_value("Content-Type") << std::endl;
        std::cout << res->body << std::endl;
    }
    else
    {
        std::cout << "generate: error code: " << res.error() << std::endl;
    }

    return 0;
}