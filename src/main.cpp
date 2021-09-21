#include <iostream>
#include <string>
#include <vector>

#define STL_READER_NO_EXCEPTIONS // functions will return false if an error occurred
#include "stl_reader.h"

#include "stl_writer.h"

#include "libslic3r/libslic3r.h"
#include "libslic3r/Point.hpp"

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
    if (!good)
    {
        std::cerr << "Couldn't load input STL " << std::endl;
        return -1;
    }

    // Extract TriangleMesh
    std::vector<Slic3r::Vec3f> points;
    std::vector<Slic3r::Vec3i> facets;

    const size_t numTris = tris.size() / 3;
    for (size_t itri = 0; itri < numTris; ++itri)
    {
        // coordinates of triangle for itri
        for (size_t icorner = 0; icorner < 3; ++icorner)
        {
            float *c = &coords[3 * tris[3 * itri + icorner]];
            points.push_back({c[0], c[1], c[2]});
        }

        facets.push_back({static_cast<int>(tris[3 * itri + 0]), static_cast<int>(tris[3 * itri + 1]), static_cast<int>(tris[3 * itri + 2])});
    }

    stl_writer::WriteStlFile("cpp-service-output.stl", coords, normals, tris);
}
