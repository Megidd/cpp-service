#ifndef TRIANGLEMESH_H
#define TRIANGLEMESH_H

#include "libslic3r/libslic3r.h"
#include "libslic3r/BoundingBox.hpp"

namespace Slic3r {

// Wrapper class arround Qt3D mesh with needed methods by IndexedMesh (and libIGL)
class TriangleMesh
{
public:
    TriangleMesh();
    TriangleMesh(const std::vector<Vec3f> &vertices, const std::vector<Vec3i> &indices);
    TriangleMesh(const std::vector<Vec3d> &vertices, const std::vector<Vec3i> &indices);

    // Methods needed by IndexedMesh (and libIGL)
    const std::vector<Vec3f>& vertices() const { return m_vertices; };
    const std::vector<Vec3i>& indices()  const { return m_indices; };
    const Vec3f& vertices(size_t idx) const { return m_vertices[idx]; };
    const Vec3i& indices(size_t idx) const { return m_indices[idx]; };

    BoundingBoxf3 bounding_box() const;
    Vec3d normal_by_face_id(int face_id) const;

    size_t facets_count() const { return m_indices.size(); }
    bool empty() const { return facets_count() == 0; }

    // TriangleMeshSlicer will need this,
    // which we don't use
    // Called inside SupportTreeBuilder::merged_mesh
    void require_shared_vertices() {/* TODO: not needed now */};

    // A method to be able to save mesh as STL easily,
    // with external tools and libraries.
    void rawDataArrays(std::vector<float> &coords,
                       std::vector<float> &normals,
                       std::vector<unsigned int> &tris);

private:
    Vec3f CrossProduct(Vec3f a, Vec3f b) const
    {
        float x = a.y() * b.z() - a.z() * b.y();
        float y = a.z() * b.x() - a.x() * b.z();
        float z = a.x() * b.y() - a.y() * b.x();
        return Vec3f{x, y, z};
    }

private:
    std::vector<Vec3f> m_vertices;
    std::vector<Vec3i> m_indices;
};

} // namespace Slic3r

#endif // TRIANGLEMESH_H
