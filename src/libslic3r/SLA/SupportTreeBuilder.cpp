#include "libslic3r/SLA/SupportTreeBuilder.hpp"
#include "libslic3r/SLA/Contour3D.hpp"
#include "libslic3r/SLA/SupportTreeMesher.hpp"

namespace Slic3r {
namespace sla {

Head::Head(double       r_big_mm,
           double       r_small_mm,
           double       length_mm,
           double       penetration,
           const Vec3d &direction,
           const Vec3d &offset)
    : dir(direction)
    , pos(offset)
    , r_back_mm(r_big_mm)
    , r_pin_mm(r_small_mm)
    , width_mm(length_mm)
    , penetration_mm(penetration)
{
}

SupportTreeBuilder::SupportTreeBuilder(SupportTreeBuilder &&o)
    : m_heads(std::move(o.m_heads))
    , m_head_indices{std::move(o.m_head_indices)}
    , m_pillars{std::move(o.m_pillars)}
    , m_bridges{std::move(o.m_bridges)}
    , m_crossbridges{std::move(o.m_crossbridges)}
    , m_pad{std::move(o.m_pad)}
    , m_meshcache{std::move(o.m_meshcache)}
    , m_meshcache_valid{o.m_meshcache_valid}
    , m_model_height{o.m_model_height}
    , ground_level{o.ground_level}
{}

SupportTreeBuilder::SupportTreeBuilder(const SupportTreeBuilder &o)
    : m_heads(o.m_heads)
    , m_head_indices{o.m_head_indices}
    , m_pillars{o.m_pillars}
    , m_bridges{o.m_bridges}
    , m_crossbridges{o.m_crossbridges}
    , m_pad{o.m_pad}
    , m_meshcache{o.m_meshcache}
    , m_meshcache_valid{o.m_meshcache_valid}
    , m_model_height{o.m_model_height}
    , ground_level{o.ground_level}
{}

SupportTreeBuilder &SupportTreeBuilder::operator=(SupportTreeBuilder &&o)
{
    m_heads = std::move(o.m_heads);
    m_head_indices = std::move(o.m_head_indices);
    m_pillars = std::move(o.m_pillars);
    m_bridges = std::move(o.m_bridges);
    m_crossbridges = std::move(o.m_crossbridges);
    m_pad = std::move(o.m_pad);
    m_meshcache = std::move(o.m_meshcache);
    m_meshcache_valid = o.m_meshcache_valid;
    m_model_height = o.m_model_height;
    ground_level = o.ground_level;
    return *this;
}

SupportTreeBuilder &SupportTreeBuilder::operator=(const SupportTreeBuilder &o)
{
    m_heads = o.m_heads;
    m_head_indices = o.m_head_indices;
    m_pillars = o.m_pillars;
    m_bridges = o.m_bridges;
    m_crossbridges = o.m_crossbridges;
    m_pad = o.m_pad;
    m_meshcache = o.m_meshcache;
    m_meshcache_valid = o.m_meshcache_valid;
    m_model_height = o.m_model_height;
    ground_level = o.ground_level;
    return *this;
}

void SupportTreeBuilder::add_pillar_base(long pid, double baseheight, double radius)
{
    std::lock_guard<Mutex> lk(m_mutex);
    assert(pid >= 0 && size_t(pid) < m_pillars.size());
    Pillar& pll = m_pillars[size_t(pid)];
    m_pedestals.emplace_back(pll.endpt, std::min(baseheight, pll.height),
                             std::max(radius, pll.r), pll.r);

    m_pedestals.back().id = m_pedestals.size() - 1;
    m_meshcache_valid = false;
}

const TriangleMesh &SupportTreeBuilder::merged_mesh(size_t steps) const
{
    if (m_meshcache_valid) return m_meshcache;

    Contour3D merged;

    for (auto &head : m_heads) {
        if (ctl().stopcondition()) break;
        if (head.is_valid()) merged.merge(get_mesh(head, steps));
    }

    for (auto &pill : m_pillars) {
        if (ctl().stopcondition()) break;
        merged.merge(get_mesh(pill, steps));
    }

    for (auto &pedest : m_pedestals) {
        if (ctl().stopcondition()) break;
        merged.merge(get_mesh(pedest, steps));
    }

    for (auto &j : m_junctions) {
        if (ctl().stopcondition()) break;
        merged.merge(get_mesh(j, steps));
    }

    for (auto &bs : m_bridges) {
        if (ctl().stopcondition()) break;
        merged.merge(get_mesh(bs, steps));
    }

    for (auto &bs : m_crossbridges) {
        if (ctl().stopcondition()) break;
        merged.merge(get_mesh(bs, steps));
    }

    for (auto &bs : m_diffbridges) {
        if (ctl().stopcondition()) break;
        merged.merge(get_mesh(bs, steps));
    }

    for (auto &anch : m_anchors) {
        if (ctl().stopcondition()) break;
        merged.merge(get_mesh(anch, steps));
    }

    if (ctl().stopcondition()) {
        // In case of failure we have to return an empty mesh
        m_meshcache = TriangleMesh();
        return m_meshcache;
    }

    m_meshcache = to_triangle_mesh(merged);

    // The mesh will be passed by const-pointer to TriangleMeshSlicer,
    // which will need this.
    if (!m_meshcache.empty()) m_meshcache.require_shared_vertices();

    BoundingBoxf3 &&bb = m_meshcache.bounding_box();
    m_model_height       = bb.max(Z) - bb.min(Z);

    m_meshcache_valid = true;
    return m_meshcache;
}

double SupportTreeBuilder::full_height() const
{
    if (merged_mesh().empty() && !pad().empty())
        return pad().cfg.full_height();

    double h = mesh_height();
    if (!pad().empty()) h += pad().cfg.required_elevation();
    return h;
}

const TriangleMesh &SupportTreeBuilder::merge_and_cleanup()
{
    // in case the mesh is not generated, it should be...
    auto &ret = merged_mesh();

    // Doing clear() does not garantee to release the memory.
    m_heads = {};
    m_head_indices = {};
    m_pillars = {};
    m_junctions = {};
    m_bridges = {};

    return ret;
}

const TriangleMesh &SupportTreeBuilder::retrieve_mesh(MeshType meshtype) const
{
    switch(meshtype) {
    case MeshType::Support: return merged_mesh();
    case MeshType::Pad:     return pad().tmesh;
    }

    return m_meshcache;
}

}} // namespace Slic3r::sla
