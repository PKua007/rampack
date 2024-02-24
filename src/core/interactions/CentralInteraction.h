//
// Created by Piotr Kubala on 22/12/2020.
//

#ifndef RAMPACK_CENTRALINTERACTION_H
#define RAMPACK_CENTRALINTERACTION_H

#include <utility>
#include <vector>
#include <functional>

#include "core/Interaction.h"


/**
 * @brief A class representing the central interaction, where the energy depends only on the distance between
 * interaction centres.
 * @details Concrete potentials are programmed by implementing CentralInteraction::calculateEnergyForDistance2 method.
 */
class CentralInteraction : public Interaction {
public:
    using CentresProvider = std::function<std::vector<Vector<3>>(const std::byte *)>;

private:
    CentresProvider centresProvider;

protected:
    /**
     * @brief Method which should be implemented for a concrete central interaction.
     * @param distance2 distance squared between interaction centres
     */
    [[nodiscard]] virtual double calculateEnergyForDistance2(double distance2) const = 0;

public:
    /**
     * @brief Constructs the interaction with a single interaction centre in the origin (an empty interaction centres
     * list)
     */
    CentralInteraction() { this->installOnSphere(); }

    /**
     * @brief Constructs the interaction with a concrete list of interaction centres @a potentialCentres.
     */
    explicit CentralInteraction(const std::vector<Vector<3>> &potentialCentres) {
        this->installOnCentres(potentialCentres);
    }

    explicit CentralInteraction(CentresProvider centresProvider) : centresProvider{std::move(centresProvider)} { }

    /**
     * @brief Installs the interaction on sphere (empties the list of interaction centres).
     */
    void installOnSphere() {
        this->centresProvider = [](const std::byte *) -> std::vector<Vector<3>> { return {}; };
    }

    /**
     * @brief Install the interaction on concrete interaction centres.
     */
    void installOnCentres(const std::vector<Vector<3>> &centres) {
        this->centresProvider = [centres](const std::byte *) { return centres; };
    }

    void installCentresProvider(CentresProvider centresProvider_) {
        this->centresProvider = std::move(centresProvider_);
    }

    void detach() {
        this->installOnSphere();
    }

    [[nodiscard]] bool hasHardPart() const final { return false; }
    [[nodiscard]] bool hasWallPart() const final { return false; }
    [[nodiscard]] bool hasSoftPart() const final { return true; }
    [[nodiscard]] bool isConvex() const final { return false; }

    [[nodiscard]] double calculateEnergyBetween(const Vector<3> &pos1,
                                                [[maybe_unused]] const Matrix<3, 3> &orientation1,
                                                [[maybe_unused]] const std::byte *data1,
                                                [[maybe_unused]] std::size_t idx1,
                                                const Vector<3> &pos2,
                                                [[maybe_unused]] const Matrix<3, 3> &orientation2,
                                                [[maybe_unused]] const std::byte *data2,
                                                [[maybe_unused]] std::size_t idx2,
                                                const BoundaryConditions &bc) const final
    {
        return this->calculateEnergyForDistance2(bc.getDistance2(pos1, pos2));
    }

    [[nodiscard]] std::vector<Vector<3>> getInteractionCentres(const std::byte *data) const final {
        return this->centresProvider(data);
    }
};


#endif //RAMPACK_CENTRALINTERACTION_H
