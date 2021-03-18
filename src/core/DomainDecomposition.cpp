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

    this->prepareDomains(dimensions, neighbourGridDivisions, range, totalRange, origin);
    this->populateDomains(packing, origin);
}

void DomainDecomposition::prepareDomains(const std::array<double, 3> &dimensions,
                                         const std::array<std::size_t, 3> &neighbourGridDivisions, double range,
                                         double totalRange, const Vector<3> &origin)
{
    for (std::size_t coord{}; coord < 3; coord++) {
        Expects(origin[coord] >= 0 && origin[coord] < dimensions[coord]);
        Expects(this->domainDivisions[coord] > 0);
        Expects(neighbourGridDivisions[coord] > 0);

        if (this->domainDivisions[coord] < 2)
            continue;

        double ngCellSize = dimensions[coord] / neighbourGridDivisions[coord];
        Expects(ngCellSize >= range);
        double wholeDomainWidth = dimensions[coord] / this->domainDivisions[coord];
        // Ghost layer is the total interaction range plus the excess size of the neighbour grid cell
        double ghostLayerWidth = totalRange - range + ngCellSize;
        Expects(ghostLayerWidth < wholeDomainWidth);

        this->regionBounds[coord].resize(this->domainDivisions[coord]);
        double previousGhostEnd = -std::numeric_limits<double>::infinity();
        for (std::size_t domainIdx{}; domainIdx < this->domainDivisions[coord]; domainIdx++) {
            double theoreticalMiddle = origin[coord] + domainIdx * wholeDomainWidth;
            // Ghost layer middle should be in the middle of the closest neighbour grit cells
            double realMiddle = ngCellSize * (std::round(theoreticalMiddle/ngCellSize - 0.5) + 0.5);

            std::size_t previousDomainIdx = (domainIdx + this->domainDivisions[coord] - 1) % domainDivisions[coord];
            double &ghostBeg = this->regionBounds[coord][previousDomainIdx].end;
            double &ghostEnd = this->regionBounds[coord][domainIdx].beg;
            ghostBeg = realMiddle - ghostLayerWidth / 2, dimensions[coord];
            ghostEnd = realMiddle + ghostLayerWidth / 2, dimensions[coord];

            ExpectsMsg(ghostBeg > previousGhostEnd,
                       "Domain of index " + std::to_string(domainIdx) + " on coord " + std::to_string(coord) + " is < 0");
            previousGhostEnd = ghostEnd;

            ghostBeg = fitPeriodically(ghostBeg, dimensions[coord]);
            ghostEnd = fitPeriodically(ghostEnd, dimensions[coord]);
        }
    }
}

void DomainDecomposition::populateDomains(const Packing &packing, const Vector<3> &origin) {
    const auto &dimensions = packing.getDimensions();
    this->particlesInRegions.resize(
        std::accumulate(this->domainDivisions.begin(), this->domainDivisions.end(), 1, std::multiplies{})
    );
    for (std::size_t particleIdx{}; particleIdx < packing.size(); particleIdx++) {
        Vector<3> pos = packing[particleIdx].getPosition();
        std::array<std::size_t, 3> coords{};
        for (std::size_t i{}; i < 3; i++) {
            double wholeDomainWidth = dimensions[i] / this->domainDivisions[i];
            int coord = std::floor((pos[i] - origin[i]) / wholeDomainWidth);
            coords[i] = (coord + this->domainDivisions[i]) % this->domainDivisions[i];
        }

        if (isVectorInActiveRegion(pos, coords))
            this->particlesInRegions[coordToIdx(coords)].push_back(particleIdx);
    }
}

std::size_t DomainDecomposition::coordToIdx(const std::array<std::size_t, 3> &coords) const {
    std::size_t idx{};
    for (std::size_t i = 0; i < 3; i++) {
        Expects(coords[i] < this->domainDivisions[i]);
        idx = this->domainDivisions[i] * idx + coords[i];
    }
    return idx;
}

bool DomainDecomposition::isVectorInActiveRegion(const Vector<3> &vector,
                                                 const std::array<std::size_t, 3> &coords) const
{
    for (std::size_t i = 0; i < 3; i++) {
        Expects(coords[i] < this->domainDivisions[i]);
        if (this->domainDivisions[i] < 2)
            continue;

        double beg = this->regionBounds[i][coords[i]].beg;
        double end = this->regionBounds[i][coords[i]].end;
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
DomainDecomposition::getActiveRegionBoundaries(const std::array<std::size_t, 3> &coords) const
{
    std::array<std::pair<double, double>, 3> boundaries;
    for (std::size_t i = 0; i < 3; i++) {
        Expects(coords[i] < this->domainDivisions[i]);
        if (this->domainDivisions[i] < 2) {
            boundaries[i].first = -std::numeric_limits<double>::infinity();
            boundaries[i].second = std::numeric_limits<double>::infinity();
        } else {
            boundaries[i].first = this->regionBounds[i][coords[i]].beg;
            boundaries[i].second = this->regionBounds[i][coords[i]].end;
        }
    }
    return boundaries;
}
