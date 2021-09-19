#ifndef SUPPORTABLEMESH_H
#define SUPPORTABLEMESH_H

#include "IndexedMesh.hpp"
#include "SupportPoint.hpp"
#include "SupportTreeConfig.hpp"
#include "PadConfig.h"

namespace Slic3r {

namespace sla {

struct SupportableMesh
{
    IndexedMesh  emesh;
    SupportPoints pts;
    SupportTreeConfig cfg;
    PadConfig     pad_cfg;

    explicit SupportableMesh(const TriangleMesh & trmsh,
                             const SupportPoints &sp,
                             const SupportTreeConfig &c)
        : emesh{trmsh}, pts{sp}, cfg{c}
    {}

    explicit SupportableMesh(const IndexedMesh   &em,
                             const SupportPoints &sp,
                             const SupportTreeConfig &c)
        : emesh{em}, pts{sp}, cfg{c}
    {}
};

}

}

#endif // SUPPORTABLEMESH_H
