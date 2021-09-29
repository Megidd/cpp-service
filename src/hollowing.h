#ifndef HOLLOWING_H
#define HOLLOWING_H

#include <string>
#include <iostream>

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

    void hollow(std::string pathMesh, std::string pathConfig, std::string pathOutputMesh)
    {
        std::cout << "Hollowing started..." << std::endl;
    }
} // namespace hollowing
#endif // HOLLOWING_H