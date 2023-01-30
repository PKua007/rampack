//
// Created by Piotr Kubala on 18/03/2021.
//

#include "ActiveDomain.h"

#include "utils/Exceptions.h"

bool ActiveDomain::isInside(const Vector<3> &position) const {
    Vector<3> positionRel = this->box.absoluteToRelative(position);
    for (std::size_t i{}; i < 3; i++) {
        const auto &boundary = this->bounds[i];
        if (boundary.end > boundary.beg) {
            if (positionRel[i] <= boundary.beg || positionRel[i] >= boundary.end)
                return false;
        } else {
            if (positionRel[i] <= boundary.beg && positionRel[i] >= boundary.end)
                return false;
        }
    }
    return true;
}

const ActiveDomain::RegionBounds &ActiveDomain::getBoundsForCoordinate(std::size_t coordI) const {
    Expects(coordI < 3);
    return this->bounds[coordI];
}

