#include "httplib.h"
#include <iostream>

int main(void)
{
    httplib::Client cli("127.0.0.1", 58080);

    std::string output_auto_points = "~/auto_points__http.json";

    // Compute auto points
    httplib::Params params_getPoints{
        {"mesh", "~/repos/cpp-service/test-input/teapot_10elev.stl"},
        {"config", "~/repos/cpp-service/test-input/config.json"},
        {"slices", "~/repos/cpp-service/test-input/slices.json"},
        {"args", "~/repos/cpp-service/test-input/autoargs.json"},
        {"output", output_auto_points}};

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

    // Generate mesh for auto points
    httplib::Params params_generate{
        {"mesh", "~/repos/cpp-service/test-input/teapot_10elev.stl"},
        {"config", "~/repos/cpp-service/test-input/config.json"},
        {"points", output_auto_points},
        {"output", "~/output_mesh__http.stl"}};

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

    // Hollow out a mesh
    httplib::Params params_hollow{
        {"mesh", "~/repos/cpp-service/test-input/teapot_10elev.stl"},
        {"config", "~/repos/cpp-service/test-input/config_hollow.json"},
        {"output", "~/output_hollowed__http.stl"}};

    if (auto res = cli.Post("/hollow", params_hollow))
    {
        std::cout << res->status << std::endl;
        std::cout << res->get_header_value("Content-Type") << std::endl;
        std::cout << res->body << std::endl;
    }
    else
    {
        std::cout << "hollow: error code: " << res.error() << std::endl;
    }

    return 0;
}