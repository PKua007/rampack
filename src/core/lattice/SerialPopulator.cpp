//
// Created by pkua on 19.05.22.
//

#include "SerialPopulator.h"
#include "utils/Assertions.h"


std::vector<Shape> SerialPopulator::populateLattice(const Lattice &lattice, std::size_t numOfShapes) const {
    Expects(lattice.size() >= numOfShapes);

    std::vector<Shape> shapes;
    shapes.reserve(numOfShapes);
    const auto &dim = lattice.getDimensions();

    std::array<std::size_t, 3> i{};
    i.fill(0);

    for (i[this->axisOrder[0]] = 0; i[this->axisOrder[0]] < dim[this->axisOrder[0]]; i[this->axisOrder[0]]++) {
        for (i[this->axisOrder[1]] = 0; i[this->axisOrder[1]] < dim[this->axisOrder[1]]; i[this->axisOrder[1]]++) {
            for (i[this->axisOrder[2]] = 0; i[this->axisOrder[2]] < dim[this->axisOrder[2]]; i[this->axisOrder[2]]++) {
                const auto &cell = lattice.getSpecificCell(i[0], i[1], i[2]);
                for (const auto &shape : cell) {
                    if (shapes.size() == numOfShapes)
                        return shapes;
                    Vector<3> pos = Vector<3>{static_cast<double>(i[0]),
                                              static_cast<double>(i[1]),
                                              static_cast<double>(i[2])};
                    pos += shape.getPosition();
                    shapes.emplace_back(cell.getBox().relativeToAbsolute(pos), shape.getOrientation());
                }
            }
        }
    }

    return shapes;
}
