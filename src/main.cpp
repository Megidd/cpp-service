#include <iostream>
#include <string>
#include <vector>

#include "supporting.h"

// // Sample system call by command-line:
//
// m3@suse15:~/repos/cpp-service/build> ./cpp-service --get-points -mesh ~/repos/cpp-service/test-server/input/teapot_10elev.stl -config ~/repos/cpp-service/test-server/input/config.json -slices ~/repos/cpp-service/test-server/input/slices.json -args ~/repos/cpp-service/test-server/input/autoargs.json
//
// m3@suse15:~/repos/cpp-service/build> ./cpp-service --generate -mesh ~/repos/cpp-service/test-server/input/teapot_10elev.stl -config ~/repos/cpp-service/test-server/input/config.json -points ~/repos/cpp-service/test-server/input/points.json

int main(int argc, char **argv)
{
    std::cout << "Logic executable started!\n";

    enum Intent
    {
        GetPoints = 1,
        Generate
    };

    // https://stackoverflow.com/a/442137/3405291
    std::vector<std::string> args(argv + 1, argv + argc);
    std::string pathMesh, pathConfig;
    std::string pathSlices, pathArgs; // Get points
    std::string pathPoints;           // Generate

    Intent intent;

    for (auto i = args.begin(); i != args.end(); ++i)
    {
        if (*i == "-h" || *i == "--help")
        {
            std::cout << "Syntax: cpp-service --get-points -mesh <stl_file> -config <json_file> -slices <json_file> -args <json_file>" << std::endl
                      << "Syntax: cpp-service --generate   -mesh <stl_file> -config <json_file> -points <json_file>" << std::endl;
            return 0;
        }
        else if (*i == "--get-points")
        {
            intent = GetPoints;
        }
        else if (*i == "--generate")
        {
            intent = Generate;
        }
        else if (*i == "-mesh")
        {
            pathMesh = *++i;
        }
        else if (*i == "-config")
        {
            pathConfig = *++i;
        }
        else if (*i == "-slices")
        {
            pathSlices = *++i;
        }
        else if (*i == "-args")
        {
            pathArgs = *++i;
        }
        else if (*i == "-points")
        {
            pathPoints = *++i;
        }
    }

    // Common for all cases:
    std::cout << "mesh == " << pathMesh << std::endl
              << "config == " << pathConfig << std::endl;

    switch (intent)
    {
    case GetPoints:
        std::cout << "Get auto points..." << std::endl;
        supporting::getPoints(pathMesh, pathConfig, pathSlices, pathArgs);
        break;
    case Generate:
        std::cout << "Generate mesh for points..." << std::endl;
        supporting::generate(pathMesh, pathConfig, pathPoints);
        break;
    default:
        std::cout << "Did you use correct system call options?" << std::endl;
    }
    return 0;
}
