#ifndef SLA_SUPPORTPOINT_HPP
#define SLA_SUPPORTPOINT_HPP

#include <vector>

#include <Eigen/Geometry>
#include "libslic3r/libslic3r.h"
#include "libslic3r/Point.hpp"

namespace Slic3r { namespace sla {

// An enum to keep track of where the current points on the ModelObject came from.
enum class PointsStatus {
    NoPoints,           // No points were generated so far.
    Generating,     // The autogeneration algorithm triggered, but not yet finished.
    AutoGenerated,  // Points were autogenerated (i.e. copied from the backend).
    UserModified    // User has done some edits.
};

struct SupportPoint
{
    Vec3f pos;
    float head_front_radius;
    bool  is_new_island;
    Vec3d normal;  // Mesh normal at ray cast hit
                   // Determines support point direction/normal
    
    SupportPoint()
        : pos(Vec3f::Zero()), head_front_radius(0.f), is_new_island(false)
    {}
    
    SupportPoint(float pos_x,
                 float pos_y,
                 float pos_z,
                 float head_radius,
                 bool  new_island = false,
                 float norm_x = 0.0f,
                 float norm_y = 0.0f,
                 float norm_z = 0.0f)
        : pos(pos_x, pos_y, pos_z)
        , head_front_radius(head_radius)
        , is_new_island(new_island)
        , normal(norm_x, norm_y, norm_z)
    {}
    
    SupportPoint(Vec3f position, float head_radius, bool new_island = false)
        : pos(position)
        , head_front_radius(head_radius)
        , is_new_island(new_island)
    {}
    
    SupportPoint(Eigen::Matrix<float, 5, 1, Eigen::DontAlign> data)
        : pos(data(0), data(1), data(2))
        , head_front_radius(data(3))
        , is_new_island(data(4) != 0.f)
    {}
    
    bool operator==(const SupportPoint &sp) const
    {
        float rdiff = std::abs(head_front_radius - sp.head_front_radius);
        return (pos == sp.pos) && rdiff < float(EPSILON) &&
               is_new_island == sp.is_new_island;
    }
    
    bool operator!=(const SupportPoint &sp) const { return !(sp == (*this)); }
    
    template<class Archive> void serialize(Archive &ar)
    {
        ar(pos, head_front_radius, is_new_island);
    }
};

using SupportPoints = std::vector<SupportPoint>;

}} // namespace Slic3r::sla

#endif // SUPPORTPOINT_HPP

