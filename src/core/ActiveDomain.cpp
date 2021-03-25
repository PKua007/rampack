//
// Created by Piotr Kubala on 18/03/2021.
//

#include "ActiveDomain.h"

#include "utils/Assertions.h"

bool ActiveDomain::isInside(const Vector<3> &position) const {
    for (std::size_t i{}; i < 3; i++) {
        const auto &boundary = this->bounds[i];
        if (boundary.end > boundary.beg) {
            if (position[i] <= boundary.beg || position[i] >= boundary.end)
                return false;
        } else {
            if (position[i] <= boundary.beg && position[i] >= boundary.end)
                return false;
        }
    }
    return true;
}

const ActiveDomain::RegionBounds &ActiveDomain::getBoundsForCoordinate(std::size_t coordI) const {
    Expects(coordI < 3);
    return this->bounds[coordI];
}

