//
// Created by Piotr Kubala on 13/03/2021.
//

#ifndef RAMPACK_DOMAINDECOMPOSITION_H
#define RAMPACK_DOMAINDECOMPOSITION_H

#include <vector>

#include "Packing.h"
#include "Interaction.h"
#include "ActiveDomain.h"
#include "TriclinicBox.h"


struct TooNarrowDomainException : public ValidationException {
private:
    std::size_t coord{};
    double wholeDomainWidth{};
    double ghostLayerWidth{};
    double ngCellSize{};

    static std::string makeWhat(std::size_t coord, double wholeDomainWidth, double ghostLayerWidth, double ngCellSize);

public:
    TooNarrowDomainException(std::size_t coord, double wholeDomainWidth, double ghostLayerWidth, double ngCellSize)
        : ValidationException(TooNarrowDomainException::makeWhat(coord, wholeDomainWidth, ghostLayerWidth, ngCellSize)),
          coord{coord}, wholeDomainWidth{wholeDomainWidth}, ghostLayerWidth{ghostLayerWidth}, ngCellSize{ngCellSize}
    { }

    [[nodiscard]] std::size_t getCoord() const { return this->coord; }
};


/**
 * @brief The class decomposes the packing space into ActiveDomain -s separated by ghost layers.
 * @details The ghost layers are aligned with neighbour grid cells and are wide enough so that the particles from
 * adjacent domains do not interact, nor they can simultaneously modify the same neighbour grid cell. In order to
 * achieve this, ghost layers minimal width is total interaction range plus neighbour grid cell side length
 */
class DomainDecomposition {
private:
    using RegionBounds = ActiveDomain::RegionBounds;

    TriclinicBox box;
    std::array<std::size_t, 3> domainDivisions{};
    std::array<std::vector<RegionBounds>, 3> regionBounds;
    std::vector<std::vector<std::size_t>> particlesInRegions;

    void prepareDomains(const std::array<std::size_t, 3> &neighbourGridDivisions, double range, double totalRange,
                        const Vector<3> &origin);
    void populateDomains(const Packing &packing, const Vector<3> &origin);
    [[nodiscard]] static double fitPeriodically(double x, double period);
    [[nodiscard]] std::size_t coordToIdx(const std::array<std::size_t, 3> &coords) const;
    [[nodiscard]] bool isRelativeVectorInActiveRegion(const Vector<3> &vector,
                                                      const std::array<std::size_t, 3> &coords) const;

public:
    /**
     * @brief It constructs the domain decomposition.
     * @param packing the packing for which the decomposition should be done
     * @param interaction the interaction between particles; it is used to calculate ghost layer width
     * @param domainDivisions an array of number of divisions in each direction: x, y and z
     * @param neighbourGridDivisions a number of neighbour grid cells (the real ones, without ghost cells) in each
     * direction
     * @param origin the middle of the first domain - due to periodic boundary conditions the domains can be placed
     * arbitrarily. However, to avoid race condition, they are aligned with the neighbour grid.
     */
    DomainDecomposition(const Packing &packing, const Interaction &interaction,
                        const std::array<std::size_t, 3> &domainDivisions,
                        const std::array<std::size_t, 3> &neighbourGridDivisions, const Vector<3> &origin);

    [[nodiscard]] const std::vector<std::size_t> &getParticlesInRegion(const std::array<std::size_t, 3> &coord) const {
        return this->particlesInRegions[this->coordToIdx(coord)];
    }

    /**
     * @brief Checks is @a vector lies within a domain with integer coordinates @a coords
     */
    [[nodiscard]] bool isVectorInActiveRegion(const Vector<3> &vector, const std::array<std::size_t, 3> &coords) const;

    /**
     * @brief Returns ActiveDomain representing the domain given by @a coords integer coordinates
     */
    [[nodiscard]] ActiveDomain getActiveDomainBounds(const std::array<std::size_t, 3> &coords) const;
};


#endif //RAMPACK_DOMAINDECOMPOSITION_H
