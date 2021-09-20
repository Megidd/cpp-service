#include <iostream>
#include <string>
#include <vector>

#define STL_READER_NO_EXCEPTIONS // functions will return false if an error occurred
#include "stl_reader.h"

int main(int argc, char **argv)
{
    std::cout << "Logic executable started!\n";

    // https://stackoverflow.com/a/442137/3405291
    std::vector<std::string> args(argv + 1, argv + argc);
    std::string pathStl, pathJson;

    for (auto i = args.begin(); i != args.end(); ++i)
    {
        if (*i == "-h" || *i == "--help")
        {
            std::cout << "Syntax: cpp-service -stl <stl_file> -json <json_file>" << std::endl;
            return 0;
        }
        else if (*i == "-stl")
        {
            pathStl = *++i;
        }
        else if (*i == "-json")
        {
            pathJson = *++i;
        }
    }

    std::cout << "STL == " << pathStl << std::endl;
    std::cout << "JSON == " << pathJson << std::endl;

    std::vector<float> coords, normals;
    std::vector<unsigned int> tris, solids;

    bool good = stl_reader::ReadStlFile(pathStl.c_str(), coords, normals, tris, solids);
    if (!good) {
        return -1;
    }

    const size_t numTris = tris.size() / 3;
    for (size_t itri = 0; itri < numTris; ++itri)
    {
        std::cout << "coordinates of triangle " << itri << ": ";
        for (size_t icorner = 0; icorner < 3; ++icorner)
        {
            float *c = &coords[3 * tris[3 * itri + icorner]];
            std::cout << "(" << c[0] << ", " << c[1] << ", " << c[2] << ") ";
        }
        std::cout << std::endl;

        float *n = &normals[3 * itri];
        std::cout << "normal of triangle " << itri << ": "
                  << "(" << n[0] << ", " << n[1] << ", " << n[2] << ")\n";
    }
}
