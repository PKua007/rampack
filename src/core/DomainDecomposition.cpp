//
// Created by Piotr Kubala on 13/03/2021.
//

#include "DomainDecomposition.h"
#include "utils/Assertions.h"


DomainDecomposition::DomainDecomposition(const Packing &packing, const Interaction &interaction,
                                         const std::array<std::size_t, 3> &domainDivisions,
                                         const std::array<std::size_t, 3> &neighbourGridDivisions,
                                         const Vector<3> &origin)
        : domainDivisions{domainDivisions}
{
    const auto &dimensions = packing.getDimensions();
    double range = interaction.getRangeRadius();
    double totalRange = interaction.getTotalRangeRadius();
    for (std::size_t i{}; i < 3; i++) {
        Expects(origin[i] >= 0 && origin[i] < dimensions[i]);
        Expects(domainDivisions[i] > 0);
        Expects(neighbourGridDivisions[i] > 0);

        if (domainDivisions[i] < 2)
            continue;

        double ngCellSize = dimensions[i] / neighbourGridDivisions[i];
        Expects(ngCellSize >= range);
        double wholeDomainWidth = dimensions[i] / domainDivisions[i];
        double ghostLayerWidth = totalRange - range + ngCellSize;
        Expects(ghostLayerWidth < wholeDomainWidth);

        this->regionBounds[i].resize(domainDivisions[i]);
        double previousEnd = -std::numeric_limits<double>::infinity();
        for (std::size_t domainIdx{}; domainIdx < domainDivisions[i]; domainIdx++) {
            double theoreticalMiddle = origin[i] + domainIdx * wholeDomainWidth;
            double realMiddle = ngCellSize * (std::round(theoreticalMiddle/ngCellSize - 0.5) + 0.5);
            auto &bounds = this->regionBounds[i][domainIdx];
            bounds.beg = realMiddle - ghostLayerWidth/2, dimensions[i];
            bounds.end = realMiddle - ghostLayerWidth/2, dimensions[i];

            ExpectsMsg(bounds.beg > previousEnd,
                       "Domain of index " + std::to_string(domainIdx) + " on coord " + std::to_string(i) + " is < 0");
            previousEnd = bounds.end;

            bounds.beg = fitPeriodically(bounds.beg, dimensions[i]);
            bounds.end = fitPeriodically(bounds.end, dimensions[domainIdx]);
        }
    }

    for (std::size_t particleIdx{}; particleIdx < packing.size(); particleIdx++) {
        Vector<3> pos = packing[particleIdx].getPosition();
        std::array<std::size_t, 3> coords{};
        for (std::size_t i{}; i < 3; i++) {
            double wholeDomainWidth = dimensions[i] / domainDivisions[i];
            int coord = std::floor((pos[i] - origin[i]) / wholeDomainWidth);
            coords[i] = (coord + domainDivisions[i]) % domainDivisions[i];
        }

        if (this->isVectorInActiveRegion(pos, coords))
            this->particlesInRegions[this->coordToIdx(coords)].push_back(particleIdx);
    }
}

std::size_t DomainDecomposition::coordToIdx(const std::array<std::size_t, 3> &coord) const {
    std::size_t idx{};
    for (std::size_t i = 0; i < 3; i++) {
        Expects(coord[i] < this->domainDivisions[i]);
        idx = this->domainDivisions[i] * idx + coord[i];
    }
    return idx;
}

bool DomainDecomposition::isVectorInActiveRegion(const Vector<3> &vector,
                                                 const std::array<std::size_t, 3> &coord) const
{
    for (std::size_t i = 0; i < 3; i++) {
        Expects(coord[i] < this->domainDivisions[i]);
        if (this->domainDivisions[i] < 2)
            continue;
        if (vector[i] <= this->regionBounds[i][coord[i]].beg || vector[i] >= this->regionBounds[i][coord[i]].end)
            return false;
    }
    return true;
}

double DomainDecomposition::fitPeriodically(double x, double period) {
    if (x < 0)
        return x + period;
    else if (x >= period)
        return x - period;
    else
        return x;
}
