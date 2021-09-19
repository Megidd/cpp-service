#include "PadConfig.h"

#include "libslic3r/libslic3r.h"
#include "libslic3r/Point.hpp"

namespace Slic3r { namespace sla {

static inline coord_t get_waffle_offset(const PadConfig &c)
{
    return scaled(c.brim_size_mm + c.wing_distance());
}

std::string PadConfig::validate() const
{
    static const double constexpr MIN_BRIM_SIZE_MM = .1;

    if (brim_size_mm < MIN_BRIM_SIZE_MM ||
        bottom_offset() > brim_size_mm + wing_distance() ||
        get_waffle_offset(*this) <= MIN_BRIM_SIZE_MM)
        return "Pad brim size is too small for the current configuration.";

    return "";
}

} } // namespace Slic3r::sla
