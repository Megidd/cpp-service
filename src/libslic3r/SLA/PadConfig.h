#ifndef PAD_H
#define PAD_H

#include <cmath> // for std::atan
#include <string> // for std::string

namespace Slic3r {

namespace sla {

struct PadConfig {
    double wall_thickness_mm = 1.;
    double wall_height_mm = 1.;
    double max_merge_dist_mm = 50;
    double wall_slope = std::atan(1.0);          // Universal constant for Pi/4
    double brim_size_mm = 1.6;

    struct EmbedObject {
        double object_gap_mm = 1.;
        double stick_stride_mm = 10.;
        double stick_width_mm = 0.5;
        double stick_penetration_mm = 0.1;
        bool enabled = false;
        bool everywhere = false;
        operator bool() const { return enabled; }
    } embed_object;

    inline PadConfig() = default;
    inline PadConfig(double thickness,
                     double height,
                     double mergedist,
                     double slope)
        : wall_thickness_mm(thickness)
        , wall_height_mm(height)
        , max_merge_dist_mm(mergedist)
        , wall_slope(slope)
    {}

    inline double bottom_offset() const
    {
        return (wall_thickness_mm + wall_height_mm) / std::tan(wall_slope);
    }

    inline double wing_distance() const
    {
        return wall_height_mm / std::tan(wall_slope);
    }

    inline double full_height() const
    {
        return wall_height_mm + wall_thickness_mm;
    }

    /// Returns the elevation needed for compensating the pad.
    inline double required_elevation() const { return wall_thickness_mm; }

    std::string validate() const;
};

} // namespace Slic3r

} // namespace sla

#endif // PAD_H
