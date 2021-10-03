#ifndef HOLLOWING_H
#define HOLLOWING_H

#include <string>
#include <iostream>

// https://github.com/AcademySoftwareFoundation/openvdb/issues/139
#define OPENVDB_8_ABI_COMPATIBLE
#include <openvdb/openvdb.h>
#include <openvdb/tools/MeshToVolume.h>
#include <openvdb/tools/LevelSetRebuild.h>

#define STL_READER_NO_EXCEPTIONS // functions will return false if an error occurred
#include "stl_reader.h"

#include "stl_writer.h"

#include "json.hpp"

namespace hollowing
{
    // Simplified mesh just to provide mesh adaptor for OVDB
    class Contour
    {
    public:
        std::vector<std::array<float, 3>> points;
        std::vector<std::array<uint, 3>> faces3;
        std::vector<std::array<uint, 4>> faces4;

        Contour() = default;
        Contour(const std::vector<std::array<float, 3>> &vertices, const std::vector<std::array<uint, 3>> &indices)
        {
            points.reserve(vertices.size());
            faces3.reserve(indices.size());

            points = vertices;
            faces3 = indices;
        }

        Contour &merge(const Contour &ctr)
        {
            using coord_t = int32_t;

            auto N = coord_t(points.size());
            auto N_f3 = faces3.size();
            auto N_f4 = faces4.size();

            points.insert(points.end(), ctr.points.begin(), ctr.points.end());
            faces3.insert(faces3.end(), ctr.faces3.begin(), ctr.faces3.end());
            faces4.insert(faces4.end(), ctr.faces4.begin(), ctr.faces4.end());

            for (size_t n = N_f3; n < faces3.size(); n++)
            {
                auto &idx = faces3[n];
                idx.at(0) += N;
                idx.at(1) += N;
                idx.at(2) += N;
            }

            for (size_t n = N_f4; n < faces4.size(); n++)
            {
                auto &idx = faces4[n];
                for (int k = 0; k < 4; k++)
                    idx.at(k) += N;
            }

            return *this;
        }

        inline bool empty() const
        {
            return points.empty() || (faces4.empty() && faces3.empty());
        }

        void scale(float factor)
        {
            for (auto &v : this->points)
            {
                v.at(0) *= factor;
                v.at(1) *= factor;
                v.at(2) *= factor;
                //v *= factor;
            }
        }

        void flip_normals()
        {
            for (std::array<uint, 3> &ind : this->faces3)
                std::swap(ind.at(0), ind.at(2));

            // https://blender.stackexchange.com/a/27699/37549
            for (std::array<uint, 4> &ind : this->faces4)
                std::swap(ind.at(0), ind.at(2));
        }
    };

    Contour loadMesh(std::string pathMesh)
    {
        // 1. STL is read by raw data arrays
        // 2. Raw data arrays are converted to smart vectors
        // 3. Smart vectors are converted to a contour

        std::vector<float> coords, normals;
        std::vector<unsigned int> tris, solids;

        bool good = stl_reader::ReadStlFile(pathMesh.c_str(), coords, normals, tris, solids);
        if (!good)
        {
            std::cerr << "Couldn't load input STL " << std::endl;
            return Contour();
        }

        std::vector<std::array<float, 3>> vertices;
        std::vector<std::array<uint, 3>> indices;

        vertices.resize(coords.size() / 3);
        indices.resize(tris.size() / 3);

        const size_t numTris = tris.size() / 3;
        for (size_t itri = 0; itri < numTris; ++itri)
        {
            // coordinates of triangle for itri
            for (size_t icorner = 0; icorner < 3; ++icorner)
            {
                const float *c = &coords[3 * tris[3 * itri + icorner]];
                // https://stackoverflow.com/a/14178307/3405291
                vertices[tris[3 * itri + icorner]] = std::array<float, 3>{{c[0], c[1], c[2]}};
            }

            indices[itri] = std::array<uint, 3>{{static_cast<uint>(tris[3 * itri + 0]),
                                                 static_cast<uint>(tris[3 * itri + 1]),
                                                 static_cast<uint>(tris[3 * itri + 2])}};
        }

        Contour contour = Contour(vertices, indices);
        return contour;
    }

    void saveMesh(Contour mesh, std::string path)
    {
        // 1. Get mesh buffers
        // 2. Compute raw data arrays out of buffers
        // 3. Save raw data arrays as STL

        const std::vector<std::array<float, 3>> &vertices = mesh.points;
        const std::vector<std::array<uint, 3>> &indices = mesh.faces3; // STL has only triangle, no other polygon.

        auto CrossProduct = [](std::array<float, 3> v1, std::array<float, 3> v2)
        {
            return std::array<float, 3>{{v1.at(1) * v2.at(2) - v1.at(2) * v2.at(1),
                                         v1.at(2) * v2.at(0) - v1.at(0) * v2.at(2),
                                         v1.at(0) * v2.at(1) - v1.at(1) * v2.at(0)}};
        };

        auto normalize = [](std::array<float, 3> v)
        {
            float lengthSquared = v.at(0) * v.at(0) + v.at(1) * v.at(1) + v.at(2) * v.at(2);
            float length = std::sqrt(lengthSquared);
            return std::array<float, 3>{{v.at(0) / length, v.at(1) / length, v.at(2) / length}};
        };

        auto normal_by_face_id = [CrossProduct, normalize, &vertices, &indices](int face_id)
        {
            auto f = indices.at(face_id);
            auto v0 = vertices.at(f.at(0));
            auto v1 = vertices.at(f.at(1));
            auto v2 = vertices.at(f.at(2));
            auto V = std::array<float, 3>{{v1.at(0) - v0.at(0), v1.at(1) - v0.at(1), v1.at(2) - v0.at(2)}};
            auto W = std::array<float, 3>{{v2.at(0) - v0.at(0), v2.at(1) - v0.at(1), v2.at(2) - v0.at(2)}};
            auto N = CrossProduct(V, W);
            N = normalize(N);
            return N;
        };

        std::vector<float> coords;
        std::vector<float> normals;
        std::vector<unsigned int> tris;
        coords.resize(vertices.size() * 3);
        normals.resize(indices.size() * 3);
        tris.resize(indices.size() * 3);
        for (int face_id = 0; face_id < indices.size(); face_id++)
        {
            auto f = indices.at(face_id);
            tris[face_id * 3 + 0] = f.at(0);
            tris[face_id * 3 + 1] = f.at(1);
            tris[face_id * 3 + 2] = f.at(2);
            auto v0 = vertices.at(f.at(0));
            coords[f.at(0) * 3 + 0] = v0.at(0);
            coords[f.at(0) * 3 + 1] = v0.at(1);
            coords[f.at(0) * 3 + 2] = v0.at(2);
            auto v1 = vertices.at(f.at(1));
            coords[f.at(1) * 3 + 0] = v1.at(0);
            coords[f.at(1) * 3 + 1] = v1.at(1);
            coords[f.at(1) * 3 + 2] = v1.at(2);
            auto v2 = vertices.at(f.at(2));
            coords[f.at(2) * 3 + 0] = v2.at(0);
            coords[f.at(2) * 3 + 1] = v2.at(1);
            coords[f.at(2) * 3 + 2] = v2.at(2);
            auto n = normal_by_face_id(face_id);
            normals[face_id * 3 + 0] = n.at(0);
            normals[face_id * 3 + 1] = n.at(1);
            normals[face_id * 3 + 2] = n.at(2);
        }

        stl_writer::WriteStlFile(path.c_str(), coords, normals, tris);
        std::cout << "mesh is saved as STL: " << path << std::endl;
    }

    struct HollowingConfig
    {
        double min_thickness = 2.;
        double quality = 0.5;
        double closing_distance = 0.5;
        bool enabled = true;
    };

    HollowingConfig loadConfig(std::string pathConfig)
    {
        // read JSON file
        std::ifstream fConfig(pathConfig.c_str());
        nlohmann::json jConfig;
        fConfig >> jConfig;
        std::cout << "config: " << jConfig << std::endl;

        HollowingConfig cfg;
        cfg.min_thickness = jConfig["min_thickness"];
        cfg.quality = jConfig["quality"];
        cfg.closing_distance = jConfig["closing_distance"];
        cfg.enabled = jConfig["enabled"];

        return cfg;
    }

    inline std::array<float, 3> to_vec3d(const openvdb::Vec3s &v) { return std::array<float, 3>{{v.x(), v.y(), v.z()}}; }
    inline std::array<uint, 3> to_vec3i(const openvdb::Vec3I &v) { return std::array<uint, 3>{uint(v[0]), uint(v[1]), uint(v[2])}; }
    inline std::array<uint, 4> to_vec4i(const openvdb::Vec4I &v) { return std::array<uint, 4>{uint(v[0]), uint(v[1]), uint(v[2]), uint(v[3])}; }

    class ContourDataAdapter
    {
    public:
        const Contour &mesh;

        size_t polygonCount() const { return mesh.faces3.size() + mesh.faces4.size(); }
        size_t pointCount() const { return mesh.points.size(); }
        size_t vertexCount(size_t n) const { return n < mesh.faces3.size() ? 3 : 4; }

        // Return position pos in local grid index space for polygon n and vertex v
        void getIndexSpacePoint(size_t n, size_t v, openvdb::Vec3d &pos) const
        {
            size_t vidx = 0;
            if (n < mesh.faces3.size())
                vidx = size_t(mesh.faces3.at(n).at(v));
            else
                vidx = size_t(mesh.faces4.at(n - mesh.faces3.size()).at(v));

            std::array<float, 3> p = mesh.points[vidx];
            pos = {p.at(0), p.at(1), p.at(2)};
        }
    };

    openvdb::FloatGrid::Ptr mesh_to_grid(
        const Contour &mesh, const openvdb::math::Transform &tr = {}, float exteriorBandWidth = 3.0f, float interiorBandWidth = 3.0f, int flags = 0)
    {
        openvdb::initialize();
        return openvdb::tools::meshToVolume<openvdb::FloatGrid>(
            ContourDataAdapter{mesh}, tr, exteriorBandWidth, interiorBandWidth,
            flags);
    }

    openvdb::FloatGrid::Ptr redistance_grid(
        const openvdb::FloatGrid &grid, double iso, double ext_range = 3., double int_range = 3.)
    {
        return openvdb::tools::levelSetRebuild(grid, float(iso), float(ext_range), float(int_range));
    }

    template <class Grid>
    Contour _volumeToMesh(
        const Grid &grid, double isovalue, double adaptivity, bool relaxDisorientedTriangles)
    {
        openvdb::initialize();

        std::vector<openvdb::Vec3s> points;
        std::vector<openvdb::Vec3I> triangles;
        std::vector<openvdb::Vec4I> quads;

        openvdb::tools::volumeToMesh(grid, points, triangles, quads, isovalue,
                                     adaptivity, relaxDisorientedTriangles);

        Contour ret;
        ret.points.reserve(points.size());
        ret.faces3.reserve(triangles.size());
        ret.faces4.reserve(quads.size());

        for (auto &v : points)
            ret.points.emplace_back(to_vec3d(v));
        for (auto &v : triangles)
            ret.faces3.emplace_back(to_vec3i(v));
        for (auto &v : quads)
            ret.faces4.emplace_back(to_vec4i(v));

        return ret;
    }

    Contour grid_to_contour(
        const openvdb::FloatGrid &grid, double isovalue, double adaptivity, bool relaxDisorientedTriangles = true)
    {
        return _volumeToMesh(grid, isovalue, adaptivity, relaxDisorientedTriangles);
    }

    Contour generate_interior(
        const Contour &mesh, double min_thickness, double voxel_scale, double closing_dist)
    {
        // Make a copy to be able to modify
        Contour imesh{mesh};

        imesh.scale(voxel_scale);

        double offset = voxel_scale * min_thickness;
        double D = voxel_scale * closing_dist;
        float out_range = 0.1f * float(offset);
        float in_range = 1.1f * float(offset + D);

        auto gridptr = mesh_to_grid(imesh, {}, out_range, in_range);

        if (closing_dist > .0)
        {
            gridptr = redistance_grid(*gridptr, -(offset + D), double(in_range));
        }
        else
        {
            D = -offset;
        }

        double iso_surface = D;
        double adaptivity = 0.;
        Contour omesh = grid_to_contour(*gridptr, iso_surface, adaptivity);

        omesh.scale(1. / voxel_scale);

        return omesh;
    }

    std::unique_ptr<Contour> generate_interior(const std::unique_ptr<Contour> &mesh,
                                               const HollowingConfig &cfg)
    {
        static const double MIN_OVERSAMPL = 3.;
        static const double MAX_OVERSAMPL = 8.;

        // I can't figure out how to increase the grid resolution through openvdb
        // API so the model will be scaled up before conversion and the result
        // scaled down. Voxels have a unit size. If I set voxelSize smaller, it
        // scales the whole geometry down, and doesn't increase the number of
        // voxels.
        //
        // max 8x upscale, min is native voxel size
        auto voxel_scale = MIN_OVERSAMPL + (MAX_OVERSAMPL - MIN_OVERSAMPL) * cfg.quality;

        std::unique_ptr<Contour> meshptr = std::make_unique<Contour>(
            generate_interior(
                *mesh.get(), cfg.min_thickness, voxel_scale, cfg.closing_distance));

        if (meshptr && !meshptr->empty())
        {
            // Commenting out normal flip generates acceptable final result
            // Porbably our polygon index order assumption is different
            // TODO: investigate the root cause
            //meshptr.get()->flip_normals();
        }

        return meshptr;
    }

    // Convert quad polygons (rectangular) to triangle polygons.
    // Only triangles are considered when saving STL.
    void quad2tri(std::unique_ptr<Contour> &mesh_ptr)
    {
        Contour *mesh = mesh_ptr.get();
        // Handle quads
        for (std::array<uint, 4> i : mesh->faces4)
        {
            // Convert one quad to two triangles
            mesh->faces3.emplace_back(std::array<uint, 3>{i.at(0), i.at(1), i.at(2)});
            mesh->faces3.emplace_back(std::array<uint, 3>{i.at(2), i.at(3), i.at(0)});
        }

        // To free up memory.
        mesh->faces4.clear();
    }

    void setOriginToBBoxCenter(std::unique_ptr<Contour> &mesh_ptr)
    {
        Contour *mesh = mesh_ptr.get();

        std::array<float, 3> min = std::array<float, 3>{FLT_MAX, FLT_MAX, FLT_MAX};
        std::array<float, 3> max = std::array<float, 3>{-FLT_MAX, -FLT_MAX, -FLT_MAX};

        int length = mesh->points.size();
        for (int i = 0; i < length; ++i)
        {

            std::array<float, 3> vertex = mesh->points.at(i);
            float x = vertex.at(0);
            float y = vertex.at(1);
            float z = vertex.at(2);

            if (x < min.at(0))
                min[0] = x;
            if (y < min.at(1))
                min[1] = y;
            if (z < min.at(2))
                min[2] = z;

            if (x > max.at(0))
                max[0] = x;
            if (y > max.at(1))
                max[1] = y;
            if (z > max.at(2))
                max[2] = z;
        }

        std::array<float, 3> meshExtents = std::array<float, 3>{max.at(0) - min.at(0), max.at(1) - min.at(1), max.at(2) - min.at(2)};
        std::array<float, 3> meshCenter = std::array<float, 3>{min.at(0) + (meshExtents.at(0) / 2.0f), min.at(1) + (meshExtents.at(1) / 2.0f), min.at(2) + (meshExtents.at(2) / 2.0f)};

        std::cout << "min ==" << min.at(0) << " , " << min.at(1) << " , " << min.at(2) << std::endl;
        std::cout << "max ==" << max.at(0) << " , " << max.at(1) << " , " << max.at(2) << std::endl;
        std::cout << "mesh extents ==" << meshExtents.at(0) << " , " << meshExtents.at(1) << " , " << meshExtents.at(2) << std::endl;
        std::cout << "mesh center ==" << meshCenter.at(0) << " , " << meshCenter.at(1) << " , " << meshCenter.at(2) << std::endl;

        // Move vertices so that bounding box center (mesh center) would move to coordinate origin (0.0f, 0.0f, 0.0f)
        for (int i = 0; i < length; ++i)
            mesh->points[i] = std::array<float, 3>{mesh->points.at(i).at(0) - meshCenter.at(0),
                                                   mesh->points.at(i).at(1) - meshCenter.at(1),
                                                   mesh->points.at(i).at(2) - meshCenter.at(2)};

        return;
    }

    void hollow(std::string pathMesh, std::string pathConfig, std::string pathOutput)
    {
        Contour input_mesh = loadMesh(pathMesh);
        HollowingConfig cfg = loadConfig(pathConfig);
        saveMesh(input_mesh, "input_mesh_to_be_hollowed.stl");
        std::cout << "Hollowing started..." << std::endl;
        std::unique_ptr<Contour> in_mesh_ptr = std::make_unique<Contour>(input_mesh);
        std::unique_ptr<Contour> out_mesh_ptr = generate_interior(in_mesh_ptr, cfg);
        quad2tri(out_mesh_ptr);
        saveMesh(*out_mesh_ptr.get(), "interior_mesh.stl");
        if (out_mesh_ptr)
            in_mesh_ptr.get()->merge(*out_mesh_ptr);
        setOriginToBBoxCenter(in_mesh_ptr);
        saveMesh(*in_mesh_ptr.get(), pathOutput);
    }

} // namespace hollowing
#endif // HOLLOWING_H
