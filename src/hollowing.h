#ifndef HOLLOWING_H
#define HOLLOWING_H

#include <string>
#include <iostream>

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

    void hollow(std::string pathMesh, std::string pathConfig, std::string pathOutputMesh)
    {
        Contour input_mesh = loadMesh(pathMesh);
        HollowingConfig cfg = loadConfig(pathConfig);
        saveMesh(input_mesh, "input_mesh_to_be_hollowed.stl");
        std::cout << "Hollowing started..." << std::endl;
    }
} // namespace hollowing
#endif // HOLLOWING_H