#include "httplib.h"
#include <iostream>

int main(void)
{
    httplib::Client cli("localhost", 58080);

    httplib::Params params{
        {"stl", "~/repos/cpp-service/test-server/input/teapot_10elev.stl"},
        {"json", "~/repos/cpp-service/test-server/input/placeholders.json"}};

    if (auto res = cli.Post("/manu-supp", params))
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