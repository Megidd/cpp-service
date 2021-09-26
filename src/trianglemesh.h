#ifndef TRIANGLEMESH_H
#define TRIANGLEMESH_H

#include "libslic3r/libslic3r.h"
#include "libslic3r/BoundingBox.hpp"

namespace Slic3r {

// Wrapper class arround mesh with needed methods by IndexedMesh (and libIGL)
class TriangleMesh
{
public:
    TriangleMesh();
    TriangleMesh(const std::vector<Vec3f> &vertices, const std::vector<Vec3i> &indices);
    TriangleMesh(const std::vector<Vec3d> &vertices, const std::vector<Vec3i> &indices);

    // Construct by raw data arrays.
    TriangleMesh(const std::vector<float> &coords, const std::vector<unsigned int> &tris,
                 float &minZ // Needed by Prusa configuration. To be computed here.
    );

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
    Vec3f CrossProduct(Vec3f v1, Vec3f v2) const
    {
        return Vec3f(v1.y() * v2.z() - v1.z() * v2.y(),
                     v1.z() * v2.x() - v1.x() * v2.z(),
                     v1.x() * v2.y() - v1.y() * v2.x());
    }

private:
    std::vector<Vec3f> m_vertices;
    std::vector<Vec3i> m_indices;
};

} // namespace Slic3r

#endif // TRIANGLEMESH_H
