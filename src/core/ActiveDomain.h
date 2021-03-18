//
// Created by Piotr Kubala on 18/03/2021.
//

#ifndef RAMPACK_ACTIVEDOMAIN_H
#define RAMPACK_ACTIVEDOMAIN_H

#include <array>

#include "geometry/Vector.h"

class ActiveDomain {
public:
    struct RegionBounds {
        double beg{};
        double end{};
    };
    
private:
    std::array<RegionBounds, 3> bounds{};

public:
    ActiveDomain() = default;
    explicit ActiveDomain(const std::array<RegionBounds, 3> &bounds) : bounds{bounds} { }

    [[nodiscard]] bool isInside(const Vector<3> &position) const;
};


#endif //RAMPACK_ACTIVEDOMAIN_H
