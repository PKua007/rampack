//
// Created by Piotr Kubala on 13/03/2021.
//

#ifndef RAMPACK_DOMAINDECOMPOSITION_H
#define RAMPACK_DOMAINDECOMPOSITION_H

#include <vector>

#include "Packing.h"
#include "Interaction.h"
#include "ActiveDomain.h"

class DomainDecomposition {
private:
    using RegionBounds = ActiveDomain::RegionBounds;

    std::array<std::size_t, 3> domainDivisions{};
    std::array<std::vector<RegionBounds>, 3> regionBounds;
    std::vector<std::vector<std::size_t>> particlesInRegions;


    void prepareDomains(const std::array<double, 3> &dimensions,
                        const std::array<std::size_t, 3> &neighbourGridDivisions, double range, double totalRange,
                        const Vector<3> &origin);
    void populateDomains(const Packing &packing, const Vector<3> &origin);
    [[nodiscard]] static double fitPeriodically(double x, double period);
    [[nodiscard]] std::size_t coordToIdx(const std::array<std::size_t, 3> &coords) const;

public:
    DomainDecomposition(const Packing &packing, const Interaction &interaction,
                        const std::array<std::size_t, 3> &domainDivisions,
                        const std::array<std::size_t, 3> &neighbourGridDivisions, const Vector<3> &origin);

    [[nodiscard]] const std::vector<std::size_t> &getParticlesInRegion(const std::array<std::size_t, 3> &coord) const {
        return this->particlesInRegions[this->coordToIdx(coord)];
    }

    [[nodiscard]] bool isVectorInActiveRegion(const Vector<3> &vector, const std::array<std::size_t, 3> &coords) const;
    [[nodiscard]] ActiveDomain getActiveDomainBounds(const std::array<std::size_t, 3> &coords) const;
};


#endif //RAMPACK_DOMAINDECOMPOSITION_H
