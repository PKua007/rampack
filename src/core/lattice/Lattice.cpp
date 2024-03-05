//
// Created by pkua on 18.05.22.
//

#include <algorithm>
#include <numeric>

#include "Lattice.h"
#include "utils/Exceptions.h"


Lattice::Lattice(const UnitCell &unitCell, const std::array<std::size_t, 3> &dimensions)
        : Lattice(unitCell.deepCopy(), dimensions)
{ }

Lattice::Lattice(UnitCell &&unitCell, const std::array<std::size_t, 3> &dimensions)
        : dimensions{dimensions}, isRegular_{true}
{
    this->cells.emplace_back(std::move(unitCell));
    Expects(std::all_of(dimensions.begin(), dimensions.end(), [](std::size_t dim) { return dim > 0; }));
    this->numCells = std::accumulate(this->dimensions.begin(), this->dimensions.end(), 1, std::multiplies<>{});
}

const UnitCell &Lattice::getSpecificCell(std::size_t i, std::size_t j, std::size_t k) const {
    if (this->isRegular_)
        return this->cells.front();
    else
        return this->cells[this->getCellIndex(i, j, k)];
}

std::vector<Shape> &Lattice::modifySpecificCellMolecules(std::size_t i, std::size_t j, std::size_t k) {
    if (this->isRegular_) {
        this->isRegular_ = false;
        this->cells.resize(this->numCells, this->cells.front());
    }

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
    if (this->isRegular_) {
        return this->numCells * this->cells.front().size();
    } else {
        std::size_t numMolecules{};
        for (const auto &cell: this->cells)
            numMolecules += cell.size();
        return numMolecules;
    }
}

std::vector<Shape> Lattice::generateMolecules() const {
    std::size_t size_ = this->size();

    std::vector<Shape> shapes;
    shapes.reserve(size_);

    for (std::size_t i{}; i < this->dimensions[0]; i++) {
        for (std::size_t j{}; j < this->dimensions[1]; j++) {
            for (std::size_t k{}; k < this->dimensions[2]; k++) {
                const auto &cell = this->getSpecificCell(i, j, k);
                for (const auto &shape : cell) {
                    Vector<3> pos = Vector<3>{static_cast<double>(i), static_cast<double>(j), static_cast<double>(k)};
                    pos += shape.getPosition();
                    shapes.emplace_back(cell.getBox().relativeToAbsolute(pos), shape.getOrientation(), shape.getData());
                }
            }
        }
    }

    return shapes;
}

const UnitCell &Lattice::getUnitCell() const {
    Expects(this->isRegular_);
    return this->cells.front();
}

const std::vector<Shape> &Lattice::getSpecificCellMolecules(std::size_t i, std::size_t j, std::size_t k) const {
    return this->getSpecificCell(i, j, k).getMolecules();
}

const std::vector<Shape> &Lattice::getUnitCellMolecules() const {
    return this->getUnitCell().getMolecules();
}

std::vector<Shape> &Lattice::modifyUnitCellMolecules() {
    Expects(this->isRegular_);
    return this->cells.front().getMolecules();
}

void Lattice::normalize() {
    if (this->isNormalized())
        return;

    if (this->isRegular_)
        this->normalizeRegular();
    else
        this->normalizeIrregular();
}

void Lattice::normalizeRegular() {
    auto &molecules = modifyUnitCellMolecules();
    for (auto &molecule : molecules) {
        Vector<3> pos = molecule.getPosition();
        for (auto &posElem : pos) {
            while (posElem < 0)
                posElem += 1;
            while (posElem >= 1)
                posElem -= 1;
        }
        molecule.setPosition(pos);
    }
}

void Lattice::normalizeIrregular() {
    const TriclinicBox &cellBox = getCellBox();
    Lattice newLattice(UnitCell(cellBox, {}), this->dimensions);
    for (auto molecule : this->generateMolecules()) {
        Vector<3> relativeBoxPos = cellBox.absoluteToRelative(molecule.getPosition());
        Vector<3> cellIDouble;
        std::transform(relativeBoxPos.begin(), relativeBoxPos.end(), cellIDouble.begin(),
                       [](double coord) { return std::floor(coord); });
        std::array<std::size_t, 3> cellI{};
        for (std::size_t i{}; i < 3; i++) {
            int dim = static_cast<int>(this->dimensions[i]);
            int coord = static_cast<int>(cellIDouble[i]);
            cellI[i] = (coord % dim + dim) % dim;
        }
        Vector<3> remainderCoord = relativeBoxPos - cellIDouble;

        auto &newMolecules = newLattice.modifySpecificCellMolecules(cellI[0], cellI[1], cellI[2]);
        molecule.setPosition(remainderCoord);
        newMolecules.push_back(molecule);
    }

    *this = newLattice;
}

bool Lattice::isNormalized() const {
    return std::all_of(this->cells.begin(), this->cells.end(), [](const auto &cell) { return cell.isNormalized(); });
}