#include <iostream>
#include <string>
#include <vector>

#define STL_READER_NO_EXCEPTIONS // functions will return false if an error occurred
#include "stl_reader.h"

#include "stl_writer.h"

#include "libslic3r/libslic3r.h"
#include "libslic3r/Point.hpp"
#include "libslic3r/SLA/SupportTreeConfig.hpp"
#include "libslic3r/SLA/SupportPoint.hpp"
#include "trianglemesh.h"
#include "libslic3r/SLA/IndexedMesh.hpp"
#include "libslic3r/SLA/SupportableMesh.h"
#include "libslic3r/SLA/SupportTreeBuilder.hpp"
#include "libslic3r/SLA/SupportTreeBuildsteps.hpp"

#include <cfloat> // for float max

#include "json.hpp"

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

    // Needed by Prusa configuration.
    float minZ = FLT_MAX;

    // Construct mesh by raw data arrays.
    Slic3r::TriangleMesh input_mesh = Slic3r::TriangleMesh(coords, tris, minZ);

    // Input raw data arrays are not needed any longer.
    // Free up memory.
    coords.clear();
    normals.clear();
    tris.clear();
    solids.clear();

    // Support tree configuration
    // TODO: UI should provide.
    Slic3r::sla::SupportTreeConfig cfg;
    cfg.object_elevation_mm = minZ;
    cfg.head_penetration_mm = 0.1f;
    cfg.head_front_radius_mm = 0.05f;
    cfg.head_back_radius_mm = 0.25f;
    cfg.head_fallback_radius_mm = 0.25f;
    cfg.head_width_mm = 2.0f; // length
    cfg.base_radius_mm = 1.5f;
    cfg.base_height_mm = 0.3f; // Thickness

    // Compute support points for the item mesh
    std::vector<Slic3r::sla::SupportPoint> support_points;

    // read JSON file
    std::ifstream fJson(pathJson.c_str());
    nlohmann::json jJson;
    fJson >> jJson;

    // iterate the array
    for (nlohmann::json::iterator it = jJson.begin(); it != jJson.end(); ++it)
    {
        std::cout << *it << std::endl;
        std::string xStr = (*it)["x"];
        std::string yStr = (*it)["y"];
        std::string zStr = (*it)["z"];
        std::string rStr = (*it)["head_front_radius"];
        float x = std::stod(xStr);
        float y = std::stod(yStr);
        float z = std::stod(zStr);
        float r = std::stod(rStr);
        Slic3r::sla::SupportPoint sp = Slic3r::sla::SupportPoint(x, y, z, r);
        support_points.emplace_back(sp);
    }

    for (auto sp : support_points)
        std::cout << " x:" << sp.pos.x() << " y:" << sp.pos.y() << " z:" << sp.pos.z() << std::endl;

    // Call the logic.
    Slic3r::sla::IndexedMesh emesh = Slic3r::sla::IndexedMesh(input_mesh);
    Slic3r::sla::SupportableMesh sm = Slic3r::sla::SupportableMesh(Slic3r::sla::SupportableMesh(emesh, support_points, cfg));
    Slic3r::sla::SupportTreeBuilder treebuilder = Slic3r::sla::SupportTreeBuilder();

    Slic3r::sla::SupportTreeBuildsteps::execute(treebuilder, sm);
    Slic3r::TriangleMesh output_mesh = treebuilder.retrieve_mesh();
    std::cout << "output mesh is returned =)" << std::endl;

    // Get output mesh raw arrays to be able to save those as STL by external tool
    std::vector<float> output_coords, output_normals;
    std::vector<unsigned int> output_tris;
    output_mesh.rawDataArrays(output_coords, output_normals, output_tris);

    // Save output mesh as STL
    stl_writer::WriteStlFile("cpp-service-output.stl", output_coords, output_normals, output_tris);
    std::cout << "output mesh is saved as STL" << std::endl;

    // Save input mesh as STL to debug
    std::vector<float> input_coords, input_normals;
    std::vector<unsigned int> input_tris;
    input_mesh.rawDataArrays(input_coords, input_normals, input_tris);
    stl_writer::WriteStlFile("cpp-service-input.stl", input_coords, input_normals, input_tris);
    std::cout << "input mesh is saved as STL to compare and debug" << std::endl;
}
