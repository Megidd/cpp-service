#ifndef SLA_BOOSTADAPTER_HPP
#define SLA_BOOSTADAPTER_HPP

#include "libslic3r/libslic3r.h"
#include "libslic3r/Point.hpp"

#include <boost/geometry.hpp>

namespace boost {
namespace geometry {
namespace traits {

/* ************************************************************************** */
/* Point concept adaptation ************************************************* */
/* ************************************************************************** */

template<> struct tag<Slic3r::Point> {
    using type = point_tag;
};

template<> struct coordinate_type<Slic3r::Point> {
    using type = coord_t;
};

template<> struct coordinate_system<Slic3r::Point> {
    using type = cs::cartesian;
};

template<> struct dimension<Slic3r::Point>: boost::mpl::int_<2> {};

template<std::size_t d> struct access<Slic3r::Point, d > {
    static inline coord_t get(Slic3r::Point const& a) {
        return a(d);
    }

    static inline void set(Slic3r::Point& a, coord_t const& value) {
        a(d) = value;
    }
};

// For Vec2d ///////////////////////////////////////////////////////////////////

template<> struct tag<Slic3r::Vec2d> {
    using type = point_tag;
};

template<> struct coordinate_type<Slic3r::Vec2d> {
    using type = double;
};

template<> struct coordinate_system<Slic3r::Vec2d> {
    using type = cs::cartesian;
};

template<> struct dimension<Slic3r::Vec2d>: boost::mpl::int_<2> {};

template<std::size_t d> struct access<Slic3r::Vec2d, d > {
    static inline double get(Slic3r::Vec2d const& a) {
        return a(d);
    }

    static inline void set(Slic3r::Vec2d& a, double const& value) {
        a(d) = value;
    }
};

// For Vec3d ///////////////////////////////////////////////////////////////////

template<> struct tag<Slic3r::Vec3d> {
    using type = point_tag;
};

template<> struct coordinate_type<Slic3r::Vec3d> {
    using type = double;
};

template<> struct coordinate_system<Slic3r::Vec3d> {
    using type = cs::cartesian;
};

template<> struct dimension<Slic3r::Vec3d>: boost::mpl::int_<3> {};

template<std::size_t d> struct access<Slic3r::Vec3d, d > {
    static inline double get(Slic3r::Vec3d const& a) {
        return a(d);
    }

    static inline void set(Slic3r::Vec3d& a, double const& value) {
        a(d) = value;
    }
};

}
}

template<> struct range_value<std::vector<Slic3r::Vec2d>> {
    using type = Slic3r::Vec2d;
};

}

#endif // SLABOOSTADAPTER_HPP
