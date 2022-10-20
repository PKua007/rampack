//
// Created by pkua on 20.10.22.
//

#include "GoldestoneTracker.h"
#include "geometry/EulerAngles.h"


void GoldestoneTracker::calculate(const Packing &packing, [[maybe_unused]] double temperature,
                                  [[maybe_unused]] double pressure, const ShapeTraits &shapeTraits)
{
    this->calculateOrigin(packing, shapeTraits);
}

std::vector<std::string> GoldestoneTracker::getIntervalHeader() const {
    std::string modeName = this->getModeName();
    std::vector<std::string> header{"_x", "_y", "_z", "_ox", "_oy", "_oz"};
    for (auto &headerItem : header)
        headerItem = modeName + headerItem;
    return header;
}

std::vector<double> GoldestoneTracker::getIntervalValues() const {
    EulerAngles eulerAngles(this->originRot);
    return {this->originPos[0], this->originPos[1], this->originPos[2],
            eulerAngles.first[0], eulerAngles.first[1], eulerAngles.first[2]};
}