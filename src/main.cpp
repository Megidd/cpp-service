#include <iostream>
#include <string>
#include <vector>

#include "supporting.h"
#include "hollowing.h"

int main(int argc, char **argv)
{
    std::cout << "Logic executable started!\n";

    enum Intent
    {
        GetPoints = 1,
        Generate,
        Hollow
    };

    // https://stackoverflow.com/a/442137/3405291
    std::vector<std::string> args(argv + 1, argv + argc);
    std::string pathMesh, pathConfig, pathOutput;
    std::string pathSlices, pathArgs; // Get points
    std::string pathPoints;           // Generate
    
    Intent intent;

    for (auto i = args.begin(); i != args.end(); ++i)
    {
        if (*i == "-h" || *i == "--help")
        {
            std::cout << "Syntax: cpp-service --get-points -mesh <stl_file> -config <json_file> -slices <json_file> -args <json_file> -output <json_file>" << std::endl
                      << "Syntax: cpp-service --generate   -mesh <stl_file> -config <json_file> -points <json_file> -output <stl_file>" << std::endl
                      << "Syntax: cpp-service --hollow     -mesh <stl_file> -config <json_file> -output <stl_file>";
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
        else if (*i == "--hollow")
        {
            intent = Hollow;
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
        else if (*i == "-output")
        {
            pathOutput = *++i;
        }
    }

    // Common for all cases:
    std::cout << "mesh == " << pathMesh << std::endl
              << "config == " << pathConfig << std::endl;

    switch (intent)
    {
    case GetPoints:
        std::cout << "Get auto points..." << std::endl;
        supporting::getPoints(pathMesh, pathConfig, pathSlices, pathArgs, pathOutput);
        break;
    case Generate:
        std::cout << "Generate mesh for points..." << std::endl;
        supporting::generate(pathMesh, pathConfig, pathPoints, pathOutput);
        break;
    case Hollow:
        std::cout << "Hollowing..." << std::endl;
        hollowing::hollow(pathMesh, pathConfig, pathOutput);
        break;
    default:
        std::cout << "Did you use correct system call options?" << std::endl;
    }
    return 0;
}
