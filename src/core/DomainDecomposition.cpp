//
// Created by Piotr Kubala on 13/03/2021.
//

#include <algorithm>
#include <numeric>

#include "DomainDecomposition.h"
#include "utils/Assertions.h"


TriclinicBox DomainDecomposition::prepareBox(const Packing &packing) {
    const auto &dimensions = packing.getDimensions();
    return TriclinicBox(Matrix<3, 3>{dimensions[0], 0, 0,
                                     0, dimensions[1], 0,
                                     0, 0, dimensions[2]});
}

DomainDecomposition::DomainDecomposition(const Packing &packing, const Interaction &interaction,
                                         const std::array<std::size_t, 3> &domainDivisions,
                                         const std::array<std::size_t, 3> &neighbourGridDivisions,
                                         const Vector<3> &origin)
        : box{DomainDecomposition::prepareBox(packing)}, domainDivisions{domainDivisions}
{
    double range = interaction.getRangeRadius();
    double totalRange = interaction.getTotalRangeRadius();

    this->prepareDomains(neighbourGridDivisions, range, totalRange, origin);
    this->populateDomains(packing, origin);
}

void DomainDecomposition::prepareDomains(const std::array<std::size_t, 3> &neighbourGridDivisions, double range,
                                         double totalRange, const Vector<3> &origin)
{
    auto boxHeights = this->box.getHeights();
    Vector<3> originRel = this->box.absoluteToRelative(origin);

    for (std::size_t coord{}; coord < 3; coord++) {
        Expects(originRel[coord] >= 0 && originRel[coord] < 1);
        Expects(this->domainDivisions[coord] > 0);
        Expects(neighbourGridDivisions[coord] > 0);

        if (this->domainDivisions[coord] < 2)
            continue;

        double ngCellSize = boxHeights[coord] / neighbourGridDivisions[coord];
        Expects(ngCellSize >= range);
        double wholeDomainWidthRel = 1. / this->domainDivisions[coord];
        // Ghost layer is the total interaction range plus the excess size of the neighbour grid cell
        double ghostLayerWidthRel = (totalRange - range + ngCellSize) / boxHeights[coord];
        Expects(ghostLayerWidthRel < wholeDomainWidthRel);


        this->regionBounds[coord].resize(this->domainDivisions[coord]);
        double previousGhostEnd = -std::numeric_limits<double>::infinity();
        for (std::size_t domainIdx{}; domainIdx < this->domainDivisions[coord]; domainIdx++) {
            double theoreticalMiddle = originRel[coord] + domainIdx * wholeDomainWidthRel;
            // Ghost layer middle should be in the middle of the closest neighbour grit cells
            double realMiddle = (std::round(theoreticalMiddle * neighbourGridDivisions[coord] - 0.5) + 0.5)
                                / neighbourGridDivisions[coord];

            std::size_t previousDomainIdx = (domainIdx + this->domainDivisions[coord] - 1) % domainDivisions[coord];
            double &ghostBeg = this->regionBounds[coord][previousDomainIdx].end;
            double &ghostEnd = this->regionBounds[coord][domainIdx].beg;
            ghostBeg = realMiddle - ghostLayerWidthRel / 2;
            ghostEnd = realMiddle + ghostLayerWidthRel / 2;

            ExpectsMsg(ghostBeg > previousGhostEnd,
                       "Domain of index " + std::to_string(domainIdx) + " on coord " + std::to_string(coord) + " is < 0");
            previousGhostEnd = ghostEnd;

            ghostBeg = fitPeriodically(ghostBeg, 1);
            ghostEnd = fitPeriodically(ghostEnd, 1);
        }
    }
}

void DomainDecomposition::populateDomains(const Packing &packing, const Vector<3> &origin) {
    this->particlesInRegions.resize(
        std::accumulate(this->domainDivisions.begin(), this->domainDivisions.end(), 1, std::multiplies{})
    );

    Vector<3> originRel = this->box.absoluteToRelative(origin);
    for (std::size_t particleIdx{}; particleIdx < packing.size(); particleIdx++) {
        Vector<3> pos = packing[particleIdx].getPosition();
        Vector<3> posRel = this->box.absoluteToRelative(pos);
        std::array<std::size_t, 3> coords{};
        for (std::size_t i{}; i < 3; i++) {
            int coord = std::floor((posRel[i] - originRel[i]) * this->domainDivisions[i]);
            coords[i] = (coord + this->domainDivisions[i]) % this->domainDivisions[i];
        }

        if (this->isRelativeVectorInActiveRegion(posRel, coords))
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

bool DomainDecomposition::isRelativeVectorInActiveRegion(const Vector<3> &vector,
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

bool DomainDecomposition::isVectorInActiveRegion(const Vector<3> &vector,
                                                 const std::array<std::size_t, 3> &coords) const
{
    return this->isRelativeVectorInActiveRegion(this->box.absoluteToRelative(vector), coords);
}

double DomainDecomposition::fitPeriodically(double x, double period) {
    if (x < 0)
        return x + period;
    else if (x >= period)
        return x - period;
    else
        return x;
}

ActiveDomain DomainDecomposition::getActiveDomainBounds(const std::array<std::size_t, 3> &coords) const {
    std::array<RegionBounds, 3> boundaries;
    for (std::size_t i = 0; i < 3; i++) {
        Expects(coords[i] < this->domainDivisions[i]);
        if (this->domainDivisions[i] < 2) {
            boundaries[i].beg = -std::numeric_limits<double>::infinity();
            boundaries[i].end = std::numeric_limits<double>::infinity();
        } else {
            boundaries[i].beg = this->regionBounds[i][coords[i]].beg;
            boundaries[i].end = this->regionBounds[i][coords[i]].end;
        }
    }
    return ActiveDomain(this->box, boundaries);
}
