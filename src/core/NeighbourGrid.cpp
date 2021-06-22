//
// Created by Michał Cieśla on 07/03/2017.
// Modified by Piotr Kubala on 26/12/2020.
//

#include <algorithm>
#include <numeric>

#include "NeighbourGrid.h"
#include "utils/Utils.h"

std::size_t NeighbourGrid::positionToCellNo(const Vector<3> &position) const {
    std::size_t result{};
    for (int i = 2; i >= 0; i--) {
        // Fix numerical accuracy problems, but only up to some EPSILON, bigger discrepancies are treated as
        // preconditions miss
        constexpr double EPSILON = std::numeric_limits<double>::epsilon() * 10;
        double posI = position[i];
        if (posI < 0) {
            Expects(posI > -EPSILON);
            posI = 0;
        } else if (posI >= this->linearSize[i]) {
            Expects(posI < this->linearSize[i] + EPSILON);
            posI = this->linearSize[i] - EPSILON;
        }

        // +1, since first row of cells on each edges is "reflected", not "real"
        std::size_t coord = static_cast<int>(posI / this->cellSize[i]) + 1;
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

std::size_t NeighbourGrid::cellNeighbourToCellNo(const std::array<std::size_t, 3> &coordinates,
                                                 const std::array<int, 3> &neighbour) const
{
    std::size_t result{};
    for (int i = 2; i >= 0; i--) {
        // -1, because neighbour array goes from 0 to 2, and real offsets are from -1 to 1
        std::size_t ix = coordinates[i] + neighbour[i] - 1;
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

int NeighbourGrid::getReflectedCellNo(std::size_t cellNo) const {
    if (!this->isCellReflected(cellNo))
        return -1;

    std::array<std::size_t, 3> coords = this->cellNoToCoordinates(cellNo);
    for (std::size_t i{}; i < 3; i++) {
        std::size_t &coord = coords[i];
        if (coord == 0)
            coord = this->cellDivisions[i] - 2;
        if (coord == this->cellDivisions[i] - 1)
            coord = 1;
    }
    return static_cast<int>(this->coordinatesToCellNo(coords));
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

NeighbourGrid::NeighbourGrid(const std::array<double, 3> &linearSize, double cellSize) {
    this->setupSizes(linearSize, cellSize);
    this->cells.resize(this->numCells);
    this->reflectedCells.resize(this->numCells);

    // Aliasing "reflected" cell lists to real ones
    for (std::size_t i{}; i < this->numCells; i++)
        this->reflectedCells[i] = this->getReflectedCellNo(i);

    this->fillNeighbouringCellsOffsets();
}

NeighbourGrid::NeighbourGrid(double linearSize, double cellSize)
        : NeighbourGrid({linearSize, linearSize, linearSize}, cellSize)
{ }

void NeighbourGrid::setupSizes(const std::array<double, 3> &newLinearSize, double newCellSize) {
    for (std::size_t i{}; i < 3; i++)
        Expects(newLinearSize[i] > 0);
    Expects(newCellSize > 0);

    // 2 additional cells on both edges - "reflected" cells - are used by periodic boundary conditions
    std::array<std::size_t, 3> cellDivisions_{};
    for (std::size_t i{}; i < 3; i++) {
        cellDivisions_[i] = static_cast<std::size_t>(floor(newLinearSize[i] / newCellSize)) + 2;
        ExpectsMsg(cellDivisions_[i] >= 3, "Neighbour grid cell too big");
    }

    this->linearSize = newLinearSize;
    this->cellDivisions = cellDivisions_;
    for (std::size_t i{}; i < 3; i++)
        this->cellSize[i] = this->linearSize[i] / static_cast<double>(this->cellDivisions[i] - 2);
    this->numCells = static_cast<std::size_t>(
        std::accumulate(this->cellDivisions.begin(), this->cellDivisions.end(), 1., std::multiplies<>{})
    );
}

void NeighbourGrid::add(std::size_t idx, const Vector<3> &position) {
    std::size_t i = this->positionToCellNo(position);
    this->getCellVector(i).push_back(idx);
}

void NeighbourGrid::remove(std::size_t idx, const Vector<3> &position) {
    std::size_t i = this->positionToCellNo(position);
    auto &cell = this->getCellVector(i);
    auto it = std::find(cell.begin(), cell.end(), idx);
    if (it != cell.end())
        cell.erase(it);
}

void NeighbourGrid::clear() {
    for (std::size_t i{}; i < this->numCells; i++)
        this->cells[i].clear();
}

const std::vector<std::size_t> &NeighbourGrid::getCell(const Vector<3> &position) const {
    std::size_t i = this->positionToCellNo(position);
    return this->getCellVector(i);
}

const std::vector<std::size_t> &NeighbourGrid::getCell(const std::array<std::size_t, 3> &coord) const {
    for (std::size_t i = 0; i < 3; i++)
        Expects(coord[i] < this->cellDivisions[i] - 2);

    return this->cells[this->realCoordinatesToCellNo(coord)];
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

std::vector<std::size_t> &NeighbourGrid::getCellVector(std::size_t cellNo) {
    if (this->reflectedCells[cellNo] == -1)
        return this->cells[cellNo];
    else
        return this->cells[this->reflectedCells[cellNo]];
}

const std::vector<std::size_t> &NeighbourGrid::getCellVector(std::size_t cellNo) const {
    if (this->reflectedCells[cellNo] == -1)
        return this->cells[cellNo];
    else
        return this->cells[this->reflectedCells[cellNo]];
}

bool NeighbourGrid::resize(const std::array<double, 3> &newLinearSize, double newCellSize) {
    std::array<std::size_t, 3> oldNumCellsInLine = this->cellDivisions;
    std::size_t oldNumCells = this->numCells;
    this->setupSizes(newLinearSize, newCellSize);

    // Early exit - if number of cells in line did not change we do not need to rebuild the structure, only clear
    if (this->cellDivisions == oldNumCellsInLine) {
        this->clear();
        return false;
    }

    // The resize is needed only if number of cells is to big for allocated memory - otherwise reuse the old structure
    if (oldNumCells < this->numCells) {
        this->cells.resize(this->numCells);
        this->reflectedCells.resize(this->numCells);
    }

    for (std::size_t i{}; i < this->numCells; i++)
        this->reflectedCells[i] = this->getReflectedCellNo(i);

    this->fillNeighbouringCellsOffsets();
    this->clear();
    return true;
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

void swap(NeighbourGrid &ng1, NeighbourGrid &ng2) {
    using std::swap;

    std::swap(ng1.linearSize, ng2.linearSize);
    std::swap(ng1.cellDivisions, ng2.cellDivisions);
    std::swap(ng1.cellSize, ng2.cellSize);
    std::swap(ng1.cells, ng2.cells);
    std::swap(ng1.reflectedCells, ng2.reflectedCells);
    std::swap(ng1.numCells, ng2.numCells);
    std::swap(ng1.neighbouringCellsOffsets, ng2.neighbouringCellsOffsets);
    std::swap(ng1.positiveNeighbouringCellsOffsets, ng2.positiveNeighbouringCellsOffsets);
}

std::array<std::size_t, 3> NeighbourGrid::getCellDivisions() const {
    return {this->cellDivisions[0] - 2, this->cellDivisions[1] - 2, this->cellDivisions[2] - 2};
}

std::size_t NeighbourGrid::getMemoryUsage() const {
    std::size_t bytes{};
    bytes += get_vector_memory_usage(this->cells);
    for (const auto &cell : this->cells)
        bytes += get_vector_memory_usage(cell);
    bytes += get_vector_memory_usage(this->reflectedCells);
    bytes += get_vector_memory_usage(this->neighbouringCellsOffsets);
    bytes += get_vector_memory_usage(this->positiveNeighbouringCellsOffsets);
    return bytes;
}
