//
// Created by pkua on 19.05.22.
//

#ifndef RAMPACK_SERIALPOPULATOR_H
#define RAMPACK_SERIALPOPULATOR_H

#include "LatticePopulator.h"
#include "LatticeTraits.h"
#include "utils/Utils.h"


class SerialPopulator : public LatticePopulator {
private:
    std::array<std::size_t, 3> axisOrder{};

public:
    explicit SerialPopulator(const std::string &axisOrderString)
            : axisOrder{LatticeTraits::parseAxisOrder(axisOrderString)}
    { }

    [[nodiscard]] std::vector<Shape> populateLattice(const Lattice &lattice, std::size_t numOfShapes) const override;
};


#endif //RAMPACK_SERIALPOPULATOR_H
