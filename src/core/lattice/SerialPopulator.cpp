//
// Created by pkua on 19.05.22.
//

#include "SerialPopulator.h"
#include "utils/Exceptions.h"


SerialPopulator::SerialPopulator(const std::string &axisOrderString, const std::size_t startFrom, const std::size_t every)
        : axisOrder{LatticeTraits::parseAxisOrder(axisOrderString)}, startFrom{startFrom}, every{every}
{
    Expects(every > 0);
}

std::vector<Shape> SerialPopulator::populateLattice(const Lattice &lattice, std::size_t numOfShapes) const {
    Expects(numOfShapes > 0);
    Expects((numOfShapes - 1) * this->every + this->startFrom < lattice.size());

    std::vector<Shape> shapes;
    shapes.reserve(numOfShapes);
    const auto &dim = lattice.getDimensions();

    std::array<std::size_t, 3> i{};
    i.fill(0);

    std::size_t shapeIdx{};
    for (i[this->axisOrder[0]] = 0; i[this->axisOrder[0]] < dim[this->axisOrder[0]]; i[this->axisOrder[0]]++) {
        for (i[this->axisOrder[1]] = 0; i[this->axisOrder[1]] < dim[this->axisOrder[1]]; i[this->axisOrder[1]]++) {
            for (i[this->axisOrder[2]] = 0; i[this->axisOrder[2]] < dim[this->axisOrder[2]]; i[this->axisOrder[2]]++) {
                const auto &cell = lattice.getSpecificCell(i[0], i[1], i[2]);
                for (const auto &shape : cell) {
                    if (shapeIdx < this->startFrom) {
                        shapeIdx++;
                        continue;
                    }
                    if ((shapeIdx - this->startFrom) % this->every != 0) {
                        shapeIdx++;
                        continue;
                    }
                    if (shapes.size() == numOfShapes)
                        return shapes;

                    auto pos = Vector<3>{static_cast<double>(i[0]),
                                         static_cast<double>(i[1]),
                                         static_cast<double>(i[2])};
                    pos += shape.getPosition();
                    shapes.emplace_back(cell.getBox().relativeToAbsolute(pos), shape.getOrientation());

                    shapeIdx++;
                }
            }
        }
    }

    return shapes;
}
