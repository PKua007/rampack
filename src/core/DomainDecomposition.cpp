//
// Created by Piotr Kubala on 13/03/2021.
//

#include <algorithm>
#include <numeric>

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
        double previousGhostEnd = -std::numeric_limits<double>::infinity();
        for (std::size_t domainIdx{}; domainIdx < domainDivisions[i]; domainIdx++) {
            double theoreticalMiddle = origin[i] + domainIdx * wholeDomainWidth;
            double realMiddle = ngCellSize * (std::round(theoreticalMiddle/ngCellSize - 0.5) + 0.5);

            std::size_t previousDomainIdx = (domainIdx + domainDivisions[i] - 1) % domainDivisions[i];
            double &ghostBeg = this->regionBounds[i][previousDomainIdx].end;
            double &ghostEnd = this->regionBounds[i][domainIdx].beg;
            ghostBeg = realMiddle - ghostLayerWidth / 2, dimensions[i];
            ghostEnd = realMiddle + ghostLayerWidth / 2, dimensions[i];

            ExpectsMsg(ghostBeg > previousGhostEnd,
                       "Domain of index " + std::to_string(domainIdx) + " on coord " + std::to_string(i) + " is < 0");
            previousGhostEnd = ghostEnd;

            ghostBeg = fitPeriodically(ghostBeg, dimensions[i]);
            ghostEnd = fitPeriodically(ghostEnd, dimensions[i]);
        }
    }

    this->particlesInRegions.resize(
        std::accumulate(domainDivisions.begin(), domainDivisions.end(), 1, std::multiplies{})
    );
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

        double beg = this->regionBounds[i][coord[i]].beg;
        double end = this->regionBounds[i][coord[i]].end;
        if (beg < end) {
            if (vector[i] <= beg || vector[i] >= end)
                return false;
        } else {
            if (vector[i] <= beg && vector[i] >= end)
                return false;
        }
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

std::array<std::pair<double, double>, 3>
DomainDecomposition::getActiveRegionBoundaries(const std::array<std::size_t, 3> &coord) const
{
    std::array<std::pair<double, double>, 3> boundaries;
    for (std::size_t i = 0; i < 3; i++) {
        Expects(coord[i] < this->domainDivisions[i]);
        if (this->domainDivisions[i] < 2) {
            boundaries[i].first = -std::numeric_limits<double>::infinity();
            boundaries[i].second = std::numeric_limits<double>::infinity();
        } else {
            boundaries[i].first = this->regionBounds[i][coord[i]].beg;
            boundaries[i].second = this->regionBounds[i][coord[i]].end;
        }
    }
    return boundaries;
}
