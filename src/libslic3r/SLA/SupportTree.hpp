#ifndef SUPPORTTREE_HPP
#define SUPPORTTREE_HPP

#include "libslic3r/SLA/SupportableMesh.h"
#include "libslic3r/SLA/JobController.hpp"

namespace Slic3r {

namespace sla {

enum class MeshType { Support, Pad };

/// The class containing mesh data for the generated supports.
class SupportTree
{
    JobController m_ctl;
public:

    // TODO: skip SupportTree::create, it depends on SupportTreeBuildsteps::execute

    virtual ~SupportTree() = default;

    virtual const TriangleMesh &retrieve_mesh(MeshType meshtype) const = 0;

    // TODO: add_pad method is skipped, due to not being critical for supporting
    // ... also it depends on ExPolygons which is avoided
    // ... ExPolygons is avoided due to its dependency on Boost and also due to
    // ... its spaghetti role


    // TODO: remove_pad method is skipped for now

    // TODO: is SupportTree::slice used in supporting?
    // ... remove it for now


    // TODO: skip retrieve_full_mesh for now

    const JobController &ctl() const { return m_ctl; }
};

} // namespace sla

} // namespace Slic3r

#endif // SUPPORTTREE_HPP
