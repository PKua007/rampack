//
// Created by Piotr Kubala on 22/12/2020.
//

#ifndef RAMPACK_CENTRALINTERACTION_H
#define RAMPACK_CENTRALINTERACTION_H

#include <utility>
#include <vector>

#include "core/Interaction.h"

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
    void installOnCentres(const std::vector<Vector<3>> &centres) { this->potentialCentres = centres; }

    [[nodiscard]] bool hasHardPart() const final { return false; }
    [[nodiscard]] bool hasSoftPart() const final { return true; }
    [[nodiscard]] double calculateEnergyBetween(const Vector<3> &shape1, const Vector<3> &shape2,
                                                const BoundaryConditions &bc) const final;
    [[nodiscard]] std::vector<Vector<3>> getInteractionCentres() const override { return this->potentialCentres; };

};


#endif //RAMPACK_CENTRALINTERACTION_H
