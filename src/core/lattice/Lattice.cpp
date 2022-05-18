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
    this->cells.resize(length, unitCell);
}

Lattice::Lattice(std::vector<UnitCell> unitCells, const std::array<std::size_t, 3> &dimensions)
        : cells{std::move(unitCells)}, dimensions{dimensions}, isRegular_{false}
{
    Expects(std::all_of(this->dimensions.begin(), this->dimensions.end(), [](std::size_t dim) { return dim > 0; }));

    std::size_t length = std::accumulate(dimensions.begin(), dimensions.end(), 1, std::multiplies<>{});
    Expects(this->cells.size() == length);
    auto sameShape = [](const auto &cell1, const auto &cell2) {
        return &(cell1.getCellShape()) == &(cell2.getCellShape());
    };
    Expects(std::equal(this->cells.begin(), this->cells.end(), this->cells.begin(), sameShape));
}

const UnitCell &Lattice::getCell(std::size_t i, std::size_t j, std::size_t k) const {
    return this->cells[this->getCellIndex(i, j, k)];
}

UnitCell &Lattice::modifyCell(std::size_t i, std::size_t j, std::size_t k) {
    this->isRegular_ = false;
    return this->cells[this->getCellIndex(i, j, k)];
}

std::size_t Lattice::getCellIndex(std::size_t i, std::size_t j, std::size_t k) const {
    Expects(i < this->dimensions[0]);
    Expects(j < this->dimensions[1]);
    Expects(k < this->dimensions[2]);

    return i + j*this->dimensions[0] + k*this->dimensions[0]*this->dimensions[1];
}

TriclinicBox Lattice::getLatticeBox() const {
    auto boxSides = this->getCellShape().getSides();
    return TriclinicBox({boxSides[0] * static_cast<double>(this->dimensions[0]),
                         boxSides[1] * static_cast<double>(this->dimensions[1]),
                         boxSides[2] * static_cast<double>(this->dimensions[2])});
}
