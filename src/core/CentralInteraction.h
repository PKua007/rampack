//
// Created by Piotr Kubala on 22/12/2020.
//

#ifndef RAMPACK_CENTRALINTERACTION_H
#define RAMPACK_CENTRALINTERACTION_H

#include <utility>
#include <vector>

#include "Interaction.h"

class CentralInteraction : public Interaction {
private:
    std::vector<Vector<3>> potentialCentres;

protected:
    [[nodiscard]] virtual double calculateEnergyForDistance(double distance) const = 0;

public:
    CentralInteraction() = default;

    explicit CentralInteraction(std::vector<Vector<3>> potentialCentres)
            : potentialCentres{std::move(potentialCentres)}
    { }

    void installOnSphere() { this->potentialCentres = {}; };
    void installOnCentres(std::vector<Vector<3>> centres) { this->potentialCentres = std::move(centres); }

    [[nodiscard]] bool hasHardPart() const override { return false; }
    [[nodiscard]] bool hasSoftPart() const override { return true; }
    [[nodiscard]] double calculateEnergyBetween(const Shape &shape1, const Shape &shape2, double scale,
                                                const BoundaryConditions &bc) const override;
    [[nodiscard]] bool overlapBetween([[maybe_unused]] const Shape &shape1, [[maybe_unused]] const Shape &shape2,
                                      [[maybe_unused]] double scale,
                                      [[maybe_unused]] const BoundaryConditions &bc) const override
    {
        return false;
    }
};


#endif //RAMPACK_CENTRALINTERACTION_H
