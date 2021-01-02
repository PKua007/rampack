//
// Created by Piotr Kubala on 22/12/2020.
//

#include <algorithm>
#include <iterator>

#include "CentralInteraction.h"

double CentralInteraction::calculateEnergyBetween([[maybe_unused]] const Vector<3> &shape1, [[maybe_unused]] const Vector<3> &shape2,
                                                  [[maybe_unused]] const BoundaryConditions &bc) const
{
    /*if (this->potentialCentres.empty())
        return this->calculateEnergyForDistance(std::sqrt(bc.getDistance2(shape1.getPosition(), shape2.getPosition())));

    std::vector<Vector<3>> centres1, centres2;
    centres1.reserve(this->potentialCentres.size());
    centres2.reserve(this->potentialCentres.size());
    std::transform(this->potentialCentres.begin(), this->potentialCentres.end(), std::back_inserter(centres1),
                   [&shape1](const auto &centre) {
                       return shape1.getPosition() + shape1.getOrientation() * centre;
                   });
    std::transform(this->potentialCentres.begin(), this->potentialCentres.end(), std::back_inserter(centres2),
                   [&shape2](const auto &centre) {
                       return shape2.getPosition() + shape2.getOrientation() * centre;
                   });

    double energy{};
    for (const auto &centre1 : centres1)
        for (const auto &centre2 : centres2)
            energy += this->calculateEnergyForDistance(std::sqrt(bc.getDistance2(centre1, centre2)));
    return energy;*/
    return 0;
}