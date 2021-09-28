#ifndef SUPPORTING_H
#define SUPPORTING_H

#include <string>

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
#include "libslic3r/ExPolygon.hpp"

#include <cfloat> // for float max

#include "json.hpp"

namespace supporting
{

    Slic3r::TriangleMesh loadMesh(std::string pathMesh, float &minZ)
    {
        std::vector<float> coords, normals;
        std::vector<unsigned int> tris, solids;

        bool good = stl_reader::ReadStlFile(pathMesh.c_str(), coords, normals, tris, solids);
        if (!good)
        {
            std::cerr << "Couldn't load input STL " << std::endl;
            return Slic3r::TriangleMesh();
        }

        // Needed by Prusa configuration.
        minZ = FLT_MAX;

        // Construct mesh by raw data arrays.
        Slic3r::TriangleMesh input_mesh = Slic3r::TriangleMesh(coords, tris, minZ);

        // Input raw data arrays are not needed any longer.
        // Free up memory.
        coords.clear();
        normals.clear();
        tris.clear();
        solids.clear();

        return input_mesh;
    }

    Slic3r::sla::SupportTreeConfig loadConfig(std::string pathConfig, const float minZ)
    {
        // Support tree configuration
        // read JSON file
        std::ifstream fConfig(pathConfig.c_str());
        nlohmann::json jConfig;
        fConfig >> jConfig;
        std::cout << "config: " << jConfig << std::endl;

        Slic3r::sla::SupportTreeConfig cfg;
        cfg.object_elevation_mm = minZ; // Just to be sure it's correct, don't rely on JSON value!
        cfg.head_penetration_mm = std::stod(std::string(jConfig["head_penetration_mm"]));
        cfg.head_front_radius_mm = std::stod(std::string(jConfig["head_front_radius_mm"]));
        cfg.head_back_radius_mm = std::stod(std::string(jConfig["head_back_radius_mm"]));
        cfg.head_fallback_radius_mm = std::stod(std::string(jConfig["head_fallback_radius_mm"]));
        cfg.head_width_mm = std::stod(std::string(jConfig["head_width_mm"])); // length
        cfg.base_radius_mm = std::stod(std::string(jConfig["base_radius_mm"]));
        cfg.base_height_mm = std::stod(std::string(jConfig["base_height_mm"])); // Thickness

        return cfg;
    }

    std::vector<Slic3r::sla::SupportPoint> loadPoints(std::string pathPoints)
    {
        // Compute support points for the item mesh
        std::vector<Slic3r::sla::SupportPoint> support_points;

        // read JSON file
        std::ifstream fPoints(pathPoints.c_str());
        nlohmann::json jPoints;
        fPoints >> jPoints;

        // iterate the array
        for (nlohmann::json::iterator it = jPoints.begin(); it != jPoints.end(); ++it)
        {
            std::cout << "point: " << *it << std::endl;
            float x = std::stod(std::string((*it)["x"]));
            float y = std::stod(std::string((*it)["y"]));
            float z = std::stod(std::string((*it)["z"]));
            float r = std::stod(std::string((*it)["head_front_radius"]));
            Slic3r::sla::SupportPoint sp = Slic3r::sla::SupportPoint(x, y, z, r);
            support_points.emplace_back(sp);
        }

        for (auto sp : support_points)
            std::cout << " x:" << sp.pos.x() << " y:" << sp.pos.y() << " z:" << sp.pos.z() << std::endl;

        return support_points;
    }

    std::vector<Slic3r::ExPolygons> loadSlices(std::string pathSlices)
    {
        std::vector<Slic3r::ExPolygons> slices;

        // read JSON file
        std::ifstream fSlices(pathSlices.c_str());
        nlohmann::json jSlices;
        fSlices >> jSlices;

        // iterate the array
        for (nlohmann::json::iterator it = jSlices["Layers"].begin(); it != jSlices["Layers"].end(); ++it)
        {
            Slic3r::ExPolygons exps;

            //std::cout << "layer: " << *it << std::endl;
            auto jExps = (*it)["Expolygons"];
            for (nlohmann::json::iterator jt = jExps.begin(); jt != jExps.end(); ++jt)
            {
                //std::cout << "contour: " << *jt << std::endl;
                auto jContour = (*jt)["Contour"];
                Slic3r::Polygon contour;
                for (nlohmann::json::iterator kt = jContour.begin(); kt != jContour.end(); ++kt)
                {
                    //std::cout << "x, y: " << *kt << std::endl;
                    double x = (*kt)["X"];
                    //// Fixed: [json.exception.type_error.302] type must be string, but is number
                    double y = (*kt)["Y"];
                    // Account for conversion from floating-point to integer
                    x /= SCALING_FACTOR;
                    y /= SCALING_FACTOR;
                    // For some reason, slicing output points are mirrored in Y direction
                    // TODO: figure out why
                    y = -y;
                    // Point class stores coordinates as integer
                    contour.points.emplace_back(Slic3r::Point(x, y));
                }

                auto jHoles = (*jt)["Holes"];
                Slic3r::Polygons holes;
                if (!jHoles.is_null())
                {
                    std::cout << "Holes is not null!" << std::endl;
                    for (nlohmann::json::iterator lt = jHoles.begin(); lt != jHoles.end(); ++lt)
                    {
                        //std::cout << "holes: " << *lt << std::endl;
                        auto jHole = (*lt)["Hole"];
                        Slic3r::Polygon hole;
                        for (nlohmann::json::iterator mt = jHole.begin(); mt != jHole.end(); ++mt)
                        {
                            //std::cout << "hole: " << *mt << std::endl;
                            //// Fixed: [json.exception.type_error.302] type must be string, but is number
                            double x = (*mt)["X"];
                            double y = (*mt)["Y"];
                            // Account for conversion from floating-point to integer
                            x /= SCALING_FACTOR;
                            y /= SCALING_FACTOR;
                            // For some reason, slicing output points are mirrored in Y direction
                            // TODO: figure out why
                            y = -y;
                            // Point class stores coordinates as integer
                            hole.points.emplace_back(Slic3r::Point(x, y));
                        }

                        // Previously, a hole was added with empty points.
                        // That was causing a runtime error.
                        // Currently, a hole is added only if its points are non-empty.
                        // Therefore fixing the runtime error.
                        if (hole.points.size() > 0)
                            holes.emplace_back(hole);
                    }
                }

                Slic3r::ExPolygon exp;

                if (holes.size() > 0)
                {
                    exp.contour = contour;
                    exp.holes = holes;
                }
                else
                    exp.contour = contour;

                exps.emplace_back(exp);
            }

            slices.emplace_back(exps);
        }

        return slices;
    }

    void getPoints(std::string pathMesh, std::string pathConfig, std::string pathSlices, std::string pathArgs)
    {
        std::cout << "Get points..." << std::endl;

        // Needed by Prusa configuration.
        float minZ = FLT_MAX;

        Slic3r::TriangleMesh input_mesh = loadMesh(pathMesh, minZ);        // minZ is computed
        Slic3r::sla::SupportTreeConfig cfg = loadConfig(pathConfig, minZ); // minZ is consumed

        // Step 1. Convert JSON to slices
        // Step 2. From slices get support_points

        std::vector<Slic3r::ExPolygons> slices = loadSlices(pathSlices);
    }

    void generate(std::string pathMesh, std::string pathConfig, std::string pathPoints)
    {
        // Needed by Prusa configuration.
        float minZ = FLT_MAX;

        Slic3r::TriangleMesh input_mesh = loadMesh(pathMesh, minZ);        // minZ is computed
        Slic3r::sla::SupportTreeConfig cfg = loadConfig(pathConfig, minZ); // minZ is consumed

        std::vector<Slic3r::sla::SupportPoint> support_points = loadPoints(pathPoints);

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

} // namespace supporting
#endif // SUPPORTING_H