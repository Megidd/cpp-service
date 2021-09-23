#include "trianglemesh.h"

#include <float.h> // for FLT_MAX on Linux

namespace Slic3r {

TriangleMesh::TriangleMesh()
{

}

TriangleMesh::TriangleMesh(const std::vector<Vec3f> &vertices, const std::vector<Vec3i> &indices)
    :
      m_vertices(vertices)
    , m_indices(indices)
{

}

TriangleMesh::TriangleMesh(const std::vector<Vec3d> &vertices, const std::vector<Vec3i> &indices)
    :
      m_vertices(std::vector<Vec3f>())
    , m_indices(indices)
{
    m_vertices.clear();
    // Just like this loop here:
    // https://github.com/prusa3d/PrusaSlicer/blob/e79382c3f6f9031bc18eabcf6ea22f00d28cc747/src/libslic3r/SLA/Contour3D.cpp#L24
    for (auto &v : vertices)
        m_vertices.emplace_back(v.cast<float>());
}

BoundingBoxf3 TriangleMesh::bounding_box() const
{
    BoundingBoxf3 bb;
    bb.defined = true;
    bb.min = {FLT_MAX, FLT_MAX, FLT_MAX};
    bb.max = {-FLT_MIN, -FLT_MIN, -FLT_MIN};
    for (Vec3f v : m_vertices) {
        if (v.x() < bb.min.x()) {
            bb.min[X] = v.x();
        }
        if (v.y() < bb.min.y()) {
            bb.min[Y] = v.y();
        }
        if (v.z() < bb.min.z()) {
            bb.min[Z] = v.z();
        }
        if (v.x() > bb.max.x()) {
            bb.max[X] = v.x();
        }
        if (v.y() > bb.max.y()) {
            bb.max[Y] = v.y();
        }
        if (v.z() > bb.max.z()) {
            bb.max[Z] = v.z();
        }
    }
    return bb;
}

Vec3d TriangleMesh::normal_by_face_id(int face_id) const
{
    Slic3r::Vec3i f = m_indices.at(face_id);
    Slic3r::Vec3f v0 = m_vertices.at(f.x());
    Slic3r::Vec3f v1 = m_vertices.at(f.y());
    Slic3r::Vec3f v2 = m_vertices.at(f.z());
    Vec3f V = v1 - v0;
    Vec3f W = v2 - v0;
    Vec3f N = CrossProduct(V, W);
    N.normalize();
    return Vec3d(static_cast<double>(N.x()), static_cast<double>(N.y()), static_cast<double>(N.z()));
}

void TriangleMesh::rawDataArrays(std::vector<float> &coords,
                                 std::vector<float> &normals,
                                 std::vector<unsigned int> &tris)
{
    coords.clear();
    normals.clear();
    tris.clear();
    coords.resize(m_vertices.size() * 3);
    normals.resize(m_indices.size() * 3);
    tris.resize(m_indices.size() * 3);
    for (int face_id = 0; face_id < m_indices.size(); face_id++)
    {
        Vec3i f = m_indices.at(face_id);
        tris[face_id * 3 + 0] = f.x();
        tris[face_id * 3 + 1] = f.y();
        tris[face_id * 3 + 2] = f.z();
        Vec3f v0 = m_vertices.at(f.x());
        coords[f.x() * 3 + 0] = v0.x();
        coords[f.x() * 3 + 1] = v0.y();
        coords[f.x() * 3 + 2] = v0.z();
        Vec3f v1 = m_vertices.at(f.y());
        coords[f.y() * 3 + 0] = v1.x();
        coords[f.y() * 3 + 1] = v1.y();
        coords[f.y() * 3 + 2] = v1.z();
        Vec3f v2 = m_vertices.at(f.z());
        coords[f.z() * 3 + 0] = v2.x();
        coords[f.z() * 3 + 1] = v2.y();
        coords[f.z() * 3 + 2] = v2.z();
        Vec3d n = normal_by_face_id(face_id);
        normals[face_id * 3 + 0] = static_cast<float>(n.x());
        normals[face_id * 3 + 1] = static_cast<float>(n.y());
        normals[face_id * 3 + 2] = static_cast<float>(n.z());
    }
}

} // namespace Slic3r
