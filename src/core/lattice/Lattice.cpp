//
// Created by pkua on 18.05.22.
//

#include <algorithm>
#include <numeric>

#include "Lattice.h"
#include "utils/Assertions.h"


Lattice::Lattice(const UnitCell &unitCell, const std::array<std::size_t, 3> &dimensions)
        : dimensions{dimensions}, isRegular_{true}
{
    Expects(std::all_of(dimensions.begin(), dimensions.end(), [](std::size_t dim) { return dim > 0; }));

    std::size_t length = std::accumulate(this->dimensions.begin(), this->dimensions.end(), 1, std::multiplies<>{});
    this->cells.reserve(length);
    for (std::size_t i{}; i < length; i++)
        this->cells.emplace_back(unitCell);
}

const UnitCell &Lattice::getCell(std::size_t i, std::size_t j, std::size_t k) const {
    return this->cells[this->getCellIndex(i, j, k)];
}

std::vector<Shape> &Lattice::modifyCellMolecules(std::size_t i, std::size_t j, std::size_t k) {
    this->isRegular_ = false;
    return this->cells[this->getCellIndex(i, j, k)].getMolecules();
}

std::size_t Lattice::getCellIndex(std::size_t i, std::size_t j, std::size_t k) const {
    Expects(i < this->dimensions[0]);
    Expects(j < this->dimensions[1]);
    Expects(k < this->dimensions[2]);

    return i + j*this->dimensions[0] + k*this->dimensions[0]*this->dimensions[1];
}

TriclinicBox Lattice::getLatticeBox() const {
    auto boxSides = this->getCellBox().getSides();
    return TriclinicBox({boxSides[0] * static_cast<double>(this->dimensions[0]),
                         boxSides[1] * static_cast<double>(this->dimensions[1]),
                         boxSides[2] * static_cast<double>(this->dimensions[2])});
}

std::size_t Lattice::size() const {
    std::size_t numMolecules{};
    for (const auto &cell : this->cells)
        numMolecules += cell.size();
    return numMolecules;
}

std::vector<Shape> Lattice::generateMolecules() const {
    std::size_t size_ = this->size();

    std::vector<Shape> shapes;
    shapes.reserve(size_);

    for (std::size_t i{}; i < this->dimensions[0]; i++) {
        for (std::size_t j{}; j < this->dimensions[1]; j++) {
            for (std::size_t k{}; k < this->dimensions[2]; k++) {
                const auto &cell = this->getCell(i, j, k);
                for (const auto &shape : cell) {
                    Vector<3> pos = Vector<3>{static_cast<double>(i), static_cast<double>(j), static_cast<double>(k)};
                    pos += shape.getPosition();
                    shapes.emplace_back(cell.getBox().relativeToAbsolute(pos), shape.getOrientation());
                }
            }
        }
    }

    return shapes;
}
