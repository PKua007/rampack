//
// Created by Michał Cieśla on 07/03/2017.
// Modified by Piotr Kubala on 26/12/2020.
//

#include <algorithm>
#include <numeric>

#include "NeighbourGrid.h"
#include "utils/Utils.h"
#include "boxes/OrthorhombicBox.h"

std::size_t NeighbourGrid::positionToCellNo(const Vector<3> &position) const {
    std::size_t result{};
    Vector<3> relativePos = this->box.absoluteToRelative(position);
    for (int i = 2; i >= 0; i--) {
        // Fix numerical accuracy problems, but only up to some EPSILON, bigger discrepancies are treated as
        // preconditions miss
        constexpr double EPSILON = std::numeric_limits<double>::epsilon() * 10;
        double relativePosI = relativePos[i];
        if (relativePosI < 0) {
            Expects(relativePosI > -EPSILON);
            relativePosI = 0;
        } else if (relativePosI >= 1) {
            Expects(relativePosI < 1 + EPSILON);
            relativePosI = 1 - EPSILON;
        }

        // +1, since first row of cells on each edges is "reflected", not "real"
        std::size_t coord = static_cast<int>(relativePosI / this->relativeCellSize[i]) + 1;
        result = this->cellDivisions[i] * result + coord;
    }
    return result;
}

std::array<std::size_t, 3> NeighbourGrid::cellNoToCoordinates(std::size_t cellNo) const {
    std::array<std::size_t, 3> result{};
    for (std::size_t i{}; i < 3; i++) {
        result[i] = cellNo % this->cellDivisions[i];
        cellNo /= this->cellDivisions[i];
    }
    return result;
}

std::size_t NeighbourGrid::coordinatesToCellNo(const std::array<std::size_t, 3> &coords) const {
    std::size_t result{};
    for (int i = 2; i >= 0; i--)
        result = this->cellDivisions[i] * result + coords[i];
    return result;
}

std::size_t NeighbourGrid::realCoordinatesToCellNo(const std::array<std::size_t, 3> &coords) const {
    std::size_t result{};
    for (int i = 2; i >= 0; i--)
        result = this->cellDivisions[i] * result + coords[i] + 1;
    return result;
}

std::size_t NeighbourGrid::cellNeighbourToCellNo(const std::array<std::size_t, 3> &coords,
                                                 const std::array<int, 3> &neighbour) const
{
    std::size_t result{};
    for (int i = 2; i >= 0; i--) {
        // -1, because neighbour array goes from 0 to 2, and real offsets are from -1 to 1
        std::size_t ix = coords[i] + neighbour[i] - 1;
        Assert(ix < this->cellDivisions[i]);
        result = this->cellDivisions[i] * result + ix;
    }
    return result;
}

bool NeighbourGrid::isCellReflected(std::size_t cellNo) const {
    std::array<std::size_t, 3> coords = this->cellNoToCoordinates(cellNo);
    for (std::size_t i{}; i < 3; i++)
        if (coords[i] == 0 || coords[i] == this->cellDivisions[i] - 1)
            return true;
    return false;
}

std::pair<std::size_t, Vector<3>> NeighbourGrid::getReflectedCellData(std::size_t cellNo) const {
    if (!this->isCellReflected(cellNo))
        return std::make_pair(cellNo, Vector<3>{});

    std::array<std::size_t, 3> coords = this->cellNoToCoordinates(cellNo);
    Vector<3> translation;
    for (std::size_t i{}; i < 3; i++) {
        std::size_t &coord = coords[i];
        if (coord == 0) {
            coord = this->cellDivisions[i] - 2;
            translation -= this->boxSides[i];
        } else if (coord == this->cellDivisions[i] - 1) {
            coord = 1;
            translation += this->boxSides[i];
        }
    }
    return std::make_pair(this->coordinatesToCellNo(coords), translation);
}

bool NeighbourGrid::increment(std::array<int, 3> &in) {
    for (std::size_t i{}; i < 3; i++) {
        in[i]++;
        if (in[i] > 2 && i < 2)
            in[i] = 0;
        else
            break;
    }
    return in[2] <= 2;
}

void NeighbourGrid::fillNeighbouringCellsOffsets() {
    // 3 x 3 neighbours
    this->neighbouringCellsOffsets.clear();
    this->neighbouringCellsOffsets.reserve(27);
    this->positiveNeighbouringCellsOffsets.clear();
    this->positiveNeighbouringCellsOffsets.reserve(13);

    // We are taking the cell somewhere in the middle and computing offsets in cell list to all of its neighbours
    std::array<int, 3> neighbour{};
    std::array<std::size_t, 3> testCellCoords{};
    neighbour.fill(0);
    for (std::size_t i{}; i < 3; i++)
        testCellCoords[i] = this->cellDivisions[i] / 2;
    std::size_t testCellNo = this->coordinatesToCellNo(testCellCoords);
    do {
        std::size_t neigbourNo = this->cellNeighbourToCellNo(testCellCoords, neighbour);
        this->neighbouringCellsOffsets.push_back(neigbourNo - testCellNo);
        if (9*neighbour[0] + 3*neighbour[1] + neighbour[2] > 13)
            this->positiveNeighbouringCellsOffsets.push_back(neigbourNo - testCellNo);
    } while(increment(neighbour));

    // sort and erase to avoid duplicates - important for small packings
    std::sort( this->neighbouringCellsOffsets.begin(), this->neighbouringCellsOffsets.end());
    this->neighbouringCellsOffsets.erase(std::unique(this->neighbouringCellsOffsets.begin(),
                                                     this->neighbouringCellsOffsets.end()),
                                         this->neighbouringCellsOffsets.end());
    std::sort( this->positiveNeighbouringCellsOffsets.begin(), this->positiveNeighbouringCellsOffsets.end());
    this->positiveNeighbouringCellsOffsets.erase(std::unique(this->positiveNeighbouringCellsOffsets.begin(),
                                                     this->positiveNeighbouringCellsOffsets.end()),
                                         this->positiveNeighbouringCellsOffsets.end());
}

NeighbourGrid::NeighbourGrid(TriclinicBox box, double cellSize, std::size_t numParticles) : box{box} {
    this->setupSizes(box, cellSize);
    this->cellHeads.resize(this->numCells);
    std::fill(this->cellHeads.begin(), this->cellHeads.end(), LIST_END);
    this->translations.resize(this->numCells);
    this->reflectedCells.resize(this->numCells);

    this->successors.resize(numParticles);
    std::fill(this->successors.begin(), this->successors.end(), NeighbourGrid::LIST_END);

    // Aliasing "reflected" cell lists to real ones
    for (std::size_t i{}; i < this->numCells; i++)
        std::tie(this->reflectedCells[i], this->translations[i]) = this->getReflectedCellData(i);

    this->fillNeighbouringCellsOffsets();
}

NeighbourGrid::NeighbourGrid(double linearSize, double cellSize, std::size_t numParticles)
        : NeighbourGrid({linearSize, linearSize, linearSize}, cellSize, numParticles)
{ }

NeighbourGrid::NeighbourGrid(const std::array<double, 3> &linearSizes, double cellSize, std::size_t numParticles)
        : NeighbourGrid(TriclinicBox(Matrix<3, 3>{linearSizes[0], 0, 0,
                                                  0, linearSizes[1], 0,
                                                  0, 0, linearSizes[2]}),
                        cellSize, numParticles)
{ }

void NeighbourGrid::setupSizes(TriclinicBox newBox, double newCellSize) {
    Expects(newBox.getVolume() > 0);
    Expects(newCellSize > 0);

    auto newBoxSides = newBox.getSides();
    auto newBoxHeights = newBox.getHeights();

    // 2 additional cells on both edges - "reflected" cells - are used by periodic boundary conditions
    std::array<std::size_t, 3> cellDivisions_{};
    for (std::size_t i{}; i < 3; i++) {
        cellDivisions_[i] = static_cast<std::size_t>(floor(newBoxHeights[i] / newCellSize)) + 2;
        ExpectsMsg(cellDivisions_[i] >= 3, "Neighbour grid cell too big");
    }

    this->box = newBox;
    this->boxSides = newBoxSides;
    this->cellDivisions = cellDivisions_;
    for (std::size_t i{}; i < 3; i++)
        this->relativeCellSize[i] = 1 / static_cast<double>(this->cellDivisions[i] - 2);
    this->numCells = static_cast<std::size_t>(
        std::accumulate(this->cellDivisions.begin(), this->cellDivisions.end(), 1., std::multiplies<>{})
    );
}

void NeighbourGrid::add(std::size_t idx, const Vector<3> &position) {
    std::size_t i = this->positionToCellNo(position);
    this->successors[idx] = this->cellHeads[i];
    this->cellHeads[i] = idx;
}

void NeighbourGrid::remove(std::size_t idx, const Vector<3> &position) {
    std::size_t i = this->positionToCellNo(position);
    std::size_t head = this->cellHeads[i];
    if (head == idx) {
        this->cellHeads[i] = this->successors[idx];
        this->successors[idx] = LIST_END;
    } else {
        while (head != LIST_END) {
            if (this->successors[head] == idx) {
                this->successors[head] = this->successors[idx];
                this->successors[idx] = LIST_END;
                break;
            }
            head = this->successors[head];
        }
    }
}

void NeighbourGrid::clear() {
    std::fill(this->cellHeads.begin(), this->cellHeads.end(), LIST_END);
    std::fill(this->successors.begin(), this->successors.end(), LIST_END);
}

NeighbourGrid::CellView NeighbourGrid::getCell(const Vector<3> &position) const {
    std::size_t i = this->positionToCellNo(position);
    std::size_t head = this->cellHeads[i];
    return CellView(*this, head);
}

NeighbourGrid::CellView NeighbourGrid::getCell(const std::array<std::size_t, 3> &coord) const {
    for (std::size_t i = 0; i < 3; i++)
        Expects(coord[i] < this->cellDivisions[i] - 2);

    std::size_t i = this->realCoordinatesToCellNo(coord);
    std::size_t head = this->cellHeads[i];
    return CellView(*this, head);
}

std::vector<std::size_t> NeighbourGrid::getNeighbours(const Vector<3> &position) const {
    std::vector<std::size_t> result;

    std::size_t cellNo = this->positionToCellNo(position);

    // Pre-counting vector capacity gives huge performance boost
    std::size_t capacity{};
    for (std::size_t cellOffset : this->neighbouringCellsOffsets)
        capacity += this->getCellVector(cellNo + cellOffset).size();
    result.reserve(capacity);

    for (std::size_t cellOffset : this->neighbouringCellsOffsets) {
        const auto &cellVector = this->getCellVector(cellNo + cellOffset);
        result.insert(result.end(), cellVector.begin(), cellVector.end());
    }
    return result;
}

std::vector<std::size_t> NeighbourGrid::getCellVector(std::size_t cellNo) const {
    std::size_t realI = this->reflectedCells[cellNo];
    std::size_t head = this->cellHeads[realI];

    std::vector<std::size_t> cell;
    while (head != NeighbourGrid::LIST_END) {
        cell.push_back(head);
        head = this->successors[head];
    }

    return cell;
}

bool NeighbourGrid::resize(TriclinicBox newBox, double newCellSize) {
    auto oldNumCellsInLine = this->cellDivisions;
    std::size_t oldNumCells = this->numCells;
    auto oldBox = this->box;
    this->setupSizes(newBox, newCellSize);

    // Early exit - if number of cells in line did not change we do not need to rebuild the structure, only clear and
    // recreate translations
    if (this->cellDivisions == oldNumCellsInLine) {
        // Very hackish way of recreating translations - rescale them using old and new boxes
        for (std::size_t i{}; i < this->numCells; i++) {
            auto &translation = this->translations[i];
            translation = newBox.relativeToAbsolute(oldBox.absoluteToRelative(translation));
        }

        this->clear();
        return false;
    }

    // The resize is needed only if number of cells is to big for allocated memory - otherwise reuse the old structure
    if (oldNumCells < this->numCells) {
        this->cellHeads.resize(this->numCells);
        this->translations.resize(this->numCells);
        this->reflectedCells.resize(this->numCells);
    }

    for (std::size_t i{}; i < this->numCells; i++)
        std::tie(this->reflectedCells[i], this->translations[i]) = this->getReflectedCellData(i);

    this->fillNeighbouringCellsOffsets();
    this->clear();
    return true;
}

bool NeighbourGrid::resize(const std::array<double, 3> &newLinearSize, double newCellSize) {
    return this->resize(TriclinicBox(Matrix<3, 3>{newLinearSize[0], 0, 0,
                                                  0, newLinearSize[1], 0,
                                                  0, 0, newLinearSize[2]}),
                        newCellSize);
}

bool NeighbourGrid::resize(double newLinearSize, double newCellSize) {
    return this->resize({newLinearSize, newLinearSize, newLinearSize}, newCellSize);
}

NeighbourGrid::NeighboursView NeighbourGrid::getNeighbouringCells(const Vector<3> &position, bool onlyPositive) const {
    if (onlyPositive)
        return NeighboursView(*this, this->positionToCellNo(position), this->positiveNeighbouringCellsOffsets);
    else
        return NeighboursView(*this, this->positionToCellNo(position), this->neighbouringCellsOffsets);
}

NeighbourGrid::NeighboursView NeighbourGrid::getNeighbouringCells(const std::array<std::size_t, 3> &coord,
                                                                  bool onlyPositive) const
{
    for (std::size_t i = 0; i < 3; i++)
        Expects(coord[i] < this->cellDivisions[i] - 2);

    if (onlyPositive)
        return NeighboursView(*this, this->realCoordinatesToCellNo(coord), this->positiveNeighbouringCellsOffsets);
    else
        return NeighboursView(*this, this->realCoordinatesToCellNo(coord), this->neighbouringCellsOffsets);
}

std::array<std::size_t, 3> NeighbourGrid::getCellDivisions() const {
    return {this->cellDivisions[0] - 2, this->cellDivisions[1] - 2, this->cellDivisions[2] - 2};
}

std::size_t NeighbourGrid::getMemoryUsage() const {
    std::size_t bytes{};
    bytes += get_vector_memory_usage(this->cellHeads);
    bytes += get_vector_memory_usage(this->translations);
    bytes += get_vector_memory_usage(this->cellHeads);
    bytes += get_vector_memory_usage(this->successors);
    bytes += get_vector_memory_usage(this->reflectedCells);
    bytes += get_vector_memory_usage(this->neighbouringCellsOffsets);
    bytes += get_vector_memory_usage(this->positiveNeighbouringCellsOffsets);
    return bytes;
}
