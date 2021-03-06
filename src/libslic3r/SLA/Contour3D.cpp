#include <libslic3r/SLA/Contour3D.hpp>
#include <libslic3r/SLA/IndexedMesh.hpp>

namespace Slic3r { namespace sla {

Contour3D::Contour3D(const TriangleMesh &trmesh)
{
    points.reserve(trmesh.vertices().size());
    faces3.reserve(trmesh.indices().size());
    
    for (auto &v : trmesh.vertices())
        points.emplace_back(v.cast<double>());
    
    std::copy(trmesh.indices().begin(), trmesh.indices().end(),
              std::back_inserter(faces3));
}

Contour3D::Contour3D(TriangleMesh &&trmesh)
{
    points.reserve(trmesh.vertices().size());
    
    for (auto &v : trmesh.vertices())
        points.emplace_back(v.cast<double>());
    
    // Throws error: binding value of type 'const vector<...>' to reference to type 'vector<...>' drops 'const' qualifier
    //faces3.swap(trmesh.indices());
    faces3 = trmesh.indices();
}

Contour3D::Contour3D(const IndexedMesh &emesh) {
    points.reserve(emesh.vertices().size());
    faces3.reserve(emesh.indices().size());
    
    for (const Vec3f& vert : emesh.vertices())
        points.emplace_back(vert.cast<double>());
    
    for (const auto& ind : emesh.indices())
        faces3.emplace_back(ind);
}

Contour3D &Contour3D::merge(const Contour3D &ctr)
{
    auto N = coord_t(points.size());
    auto N_f3 = faces3.size();
    auto N_f4 = faces4.size();
    
    points.insert(points.end(), ctr.points.begin(), ctr.points.end());
    faces3.insert(faces3.end(), ctr.faces3.begin(), ctr.faces3.end());
    faces4.insert(faces4.end(), ctr.faces4.begin(), ctr.faces4.end());
    
    for(size_t n = N_f3; n < faces3.size(); n++) {
        auto& idx = faces3[n]; idx.x() += N; idx.y() += N; idx.z() += N;
    }
    
    for(size_t n = N_f4; n < faces4.size(); n++) {
        auto& idx = faces4[n]; for (int k = 0; k < 4; k++) idx(k) += N;
    }        
    
    return *this;
}

Contour3D &Contour3D::merge(const Pointf3s &triangles)
{
    const size_t offs = points.size();
    points.insert(points.end(), triangles.begin(), triangles.end());
    faces3.reserve(faces3.size() + points.size() / 3);
    
    for(int i = int(offs); i < int(points.size()); i += 3)
        faces3.emplace_back(i, i + 1, i + 2);
    
    return *this;
}

void Contour3D::to_obj(std::ostream &stream)
{
    for(auto& p : points)
        stream << "v " << p.transpose() << "\n";
    
    for(auto& f : faces3) 
        stream << "f " << (f + Vec3i(1, 1, 1)).transpose() << "\n";
    
    for(auto& f : faces4)
        stream << "f " << (f + Vec4i(1, 1, 1, 1)).transpose() << "\n";
}

TriangleMesh to_triangle_mesh(const Contour3D &ctour) {
    if (ctour.faces4.empty()) return {ctour.points, ctour.faces3};
    
    std::vector<Vec3i> triangles;
    
    triangles.reserve(ctour.faces3.size() + 2 * ctour.faces4.size());
    std::copy(ctour.faces3.begin(), ctour.faces3.end(),
              std::back_inserter(triangles));
    
    for (auto &quad : ctour.faces4) {
        triangles.emplace_back(quad(0), quad(1), quad(2));
        triangles.emplace_back(quad(2), quad(3), quad(0));
    }
    
    return {ctour.points, std::move(triangles)};
}

TriangleMesh to_triangle_mesh(Contour3D &&ctour) {
    if (ctour.faces4.empty())
        return {std::move(ctour.points), std::move(ctour.faces3)};
    
    std::vector<Vec3i> triangles;
    
    triangles.reserve(ctour.faces3.size() + 2 * ctour.faces4.size());
    std::copy(ctour.faces3.begin(), ctour.faces3.end(),
              std::back_inserter(triangles));
    
    for (auto &quad : ctour.faces4) {
        triangles.emplace_back(quad(0), quad(1), quad(2));
        triangles.emplace_back(quad(2), quad(3), quad(0));
    }
    
    return {std::move(ctour.points), std::move(triangles)};
}

}} // namespace Slic3r::sla
