//
// Created by Piotr Kubala on 18/03/2021.
//

#ifndef RAMPACK_ACTIVEDOMAIN_H
#define RAMPACK_ACTIVEDOMAIN_H

#include <array>
#include <tuple>

#include "geometry/Vector.h"

class ActiveDomain {
public:
    struct RegionBounds {
        double beg{};
        double end{};

        friend bool operator==(const RegionBounds &lhs, const RegionBounds &rhs) {
            return std::tie(lhs.beg, lhs.end) == std::tie(rhs.beg, rhs.end);
        }

        friend bool operator!=(const RegionBounds &lhs, const RegionBounds &rhs) {
            return !(rhs == lhs);
        }
    };
    
private:
    std::array<RegionBounds, 3> bounds{};

public:
    ActiveDomain() = default;
    explicit ActiveDomain(const std::array<RegionBounds, 3> &bounds) : bounds{bounds} { }

    [[nodiscard]] bool isInside(const Vector<3> &position) const;
    [[nodiscard]] const RegionBounds &getBoundsForCoordinate(std::size_t coordI) const;
};


#endif //RAMPACK_ACTIVEDOMAIN_H
