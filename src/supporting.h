#ifndef SUPPORTING_H
#define SUPPORTING_H

#include <string>
#include <random>
#include <iomanip> // for std::setw

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
#include "libslic3r/SLA/SupportPointGenerator.hpp"

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
            //std::cout << "point: " << *it << std::endl;
            float x = std::stod(std::string((*it)["x"]));
            float y = std::stod(std::string((*it)["y"]));
            float z = std::stod(std::string((*it)["z"]));
            float r = std::stod(std::string((*it)["head_front_radius"]));
            Slic3r::sla::SupportPoint sp = Slic3r::sla::SupportPoint(x, y, z, r);
            support_points.emplace_back(sp);
        }

        // for (auto sp : support_points)
        //     std::cout << " x:" << sp.pos.x() << " y:" << sp.pos.y() << " z:" << sp.pos.z() << std::endl;

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
                    //std::cout << "Holes not null!" << std::endl;
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

    struct Args
    {
        float density = 0.0f;
        float layer_height = 0.0f;
        float min_dist = 0.0f;
        float zMax = 0.0f;
        float zMin = 0.0f;
    };

    Args loadArgs(std::string pathArgs)
    {
        Args args;
        // read JSON file
        std::ifstream fArgs(pathArgs.c_str());
        nlohmann::json jArgs;
        fArgs >> jArgs;
        std::cout << "auto args: " << jArgs << std::endl;

        args.density = std::stod(std::string(jArgs["density"]));
        args.layer_height = std::stod(std::string(jArgs["layer_height"]));
        args.min_dist = std::stod(std::string(jArgs["min_dist"]));
        args.zMax = std::stod(std::string(jArgs["zMax"]));
        args.zMin = std::stod(std::string(jArgs["zMin"]));

        return args;
    }

    void savePoints(const std::vector<Slic3r::sla::SupportPoint> &support_points, std::string pathOutputPoints)
    {
        nlohmann::json jPoints = nlohmann::json::array();
        for (auto point : support_points)
        {
            nlohmann::json jPoint = nlohmann::json::object();
            jPoint["x"] = std::to_string(point.pos.x());
            jPoint["y"] = std::to_string(point.pos.y());
            jPoint["z"] = std::to_string(point.pos.z());
            jPoint["head_front_radius"] = std::to_string(point.head_front_radius);
            jPoints.push_back(jPoint);
        }

        // write prettified JSON to file
        std::ofstream o(pathOutputPoints);
        o << std::setw(4) << jPoints << std::endl;
    }

    void getPoints(std::string pathMesh, std::string pathConfig, std::string pathSlices, std::string pathArgs, std::string pathOutputPoints)
    {
        std::cout << "Get points..." << std::endl;

        // Needed by Prusa configuration.
        float minZ = FLT_MAX;

        Slic3r::TriangleMesh input_mesh = loadMesh(pathMesh, minZ);        // minZ is computed
        Slic3r::sla::SupportTreeConfig cfg = loadConfig(pathConfig, minZ); // minZ is consumed

        // Step 1. Convert JSON to slices
        // Step 2. From slices get support_points

        std::vector<Slic3r::ExPolygons> slices = loadSlices(pathSlices);
        Args args = loadArgs(pathArgs);
        std::vector<float> slicegrid = Slic3r::grid(args.zMin, args.zMax, args.layer_height);

        assert(slices.size() == slicegrid.size() &&
               "Assert message: slices and heights should have the same size!");

        // Difference should actually be zero.
        // But consider a few percents of error due to float type and other unexpected things?
        assert(std::abs(minZ - args.zMin) < 0.01f * std::abs(minZ) &&
               "Assert message: are you sure minimum Z is correct?");

        // Create the support point generator
        Slic3r::sla::SupportPointGenerator::Config autogencfg;
        autogencfg.head_diameter = float(2 * cfg.head_front_radius_mm);
        // If density is from 0 to 200, then it is divided by 100 (rival does)
        // Our UI is from from 1 to 20, so it is divided by 10
        autogencfg.density_relative = float(args.density / 10.0f);
        autogencfg.minimal_distance = float(args.min_dist);
        Slic3r::sla::IndexedMesh emesh = Slic3r::sla::IndexedMesh(input_mesh);
        Slic3r::sla::SupportPointGenerator point_gen{emesh, autogencfg, [] {}, [](int) {}};

        // Want deterministic behavior? No?
        // Make the output repeatable? No?
        //point_gen.seed(0);
        std::random_device rd;
        point_gen.seed(rd());

        point_gen.execute(slices, slicegrid);

        // Get the calculated support points.
        std::vector<Slic3r::sla::SupportPoint> support_points = point_gen.output();
        savePoints(support_points, pathOutputPoints);
        std::cout << "output points are saved as JSON ;)" << std::endl;
    }

    void generate(std::string pathMesh, std::string pathConfig, std::string pathPoints, std::string pathOutputMesh)
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
        stl_writer::WriteStlFile(pathOutputMesh.c_str(), output_coords, output_normals, output_tris);
        std::cout << "output mesh is saved as STL" << std::endl;

        //// Save input mesh as STL to debug
        // std::vector<float> input_coords, input_normals;
        // std::vector<unsigned int> input_tris;
        // input_mesh.rawDataArrays(input_coords, input_normals, input_tris);
        // stl_writer::WriteStlFile("cpp-service-input.stl", input_coords, input_normals, input_tris);
        // std::cout << "input mesh is saved as STL to compare and debug" << std::endl;
    }

} // namespace supporting
#endif // SUPPORTING_H