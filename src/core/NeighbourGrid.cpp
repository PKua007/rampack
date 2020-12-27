//
// Created by Michał Cieśla on 07/03/2017.
// Modified by Piotr Kubala on 26/12/2020.
//

#include "NeighbourGrid.h"

std::size_t NeighbourGrid::positionToCellNo(const Vector<3> &position) const {
    std::size_t result{};
    for (int i = 2; i >= 0; i--) {
        Expects(position[i] >= 0);
        Expects(position[i] < this->linearSize);

        // +1, since first row of cells on each edges is "reflected", not "real"
        std::size_t coord = static_cast<int>(position[i] / this->cellSize) + 1;
        result = this->numOfRealCells * result + coord;
    }
    return result;
}

std::array<std::size_t, 3> NeighbourGrid::cellNoToCoordinates(std::size_t cellNo) const {
    std::array<std::size_t, 3> result{};
    for (std::size_t i{}; i < 3; i++) {
        result[i] = cellNo % this->numOfRealCells;
        cellNo /= this->numOfRealCells;
    }
    return result;
}

std::size_t NeighbourGrid::coordinatesToCellNo(const std::array<std::size_t, 3> &coords) const {
    std::size_t result{};
    for (int i = 2; i >= 0; i--)
        result = this->numOfRealCells * result + coords[i];
    return result;
}

std::size_t NeighbourGrid::cellNeighbourToCellNo(const std::array<std::size_t, 3> &coordinates,
                                                 const std::array<int, 3> &neighbour) const
{
    std::size_t result{};
    for (int i = 2; i >= 0; i--) {
        // -1, because neighbour array goes from 0 to 2, and real offsets are from -1 to 1
        std::size_t ix = coordinates[i] + neighbour[i] - 1;
        Assert(ix < this->numOfRealCells);
        result = this->numOfRealCells * result + ix;
    }
    return result;
}

bool NeighbourGrid::isCellReflected(std::size_t cellNo) const {
    std::array<std::size_t, 3> coords = this->cellNoToCoordinates(cellNo);
    return std::any_of(coords.begin(), coords.end(), [this](auto coord) {
        return coord == 0 || coord == this->numOfRealCells - 1;
    });
}

int NeighbourGrid::getReflectedCellVector(std::size_t cellNo) const {
    if (!this->isCellReflected(cellNo))
        return -1;

    std::array<std::size_t, 3> coords = this->cellNoToCoordinates(cellNo);
    for (auto &coord : coords) {
        if (coord == 0)
            coord = this->numOfRealCells - 2;
        if (coord == this->numOfRealCells - 1)
            coord = 1;
    }
    return this->coordinatesToCellNo(coords);
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
    this->neighbouringCellsOffsets.reserve(27);

    // We are taking the cell somewhere in the middle and computing offsets in cell list to all of its neighbours
    std::array<int, 3> neighbour{};
    std::array<std::size_t, 3> testCellCoords{};
    neighbour.fill(0);
    testCellCoords.fill(this->numOfRealCells / 2);
    std::size_t testCellNo = this->coordinatesToCellNo(testCellCoords);
    do {
        std::size_t neigbourNo = this->cellNeighbourToCellNo(testCellCoords, neighbour);
        this->neighbouringCellsOffsets.push_back(neigbourNo - testCellNo);
    } while(increment(neighbour));

    // sort and erase to avoid duplicates - important for small packings
    std::sort( this->neighbouringCellsOffsets.begin(), this->neighbouringCellsOffsets.end());
    this->neighbouringCellsOffsets.erase(std::unique(this->neighbouringCellsOffsets.begin(),
                                                     this->neighbouringCellsOffsets.end()),
                                         this->neighbouringCellsOffsets.end());
}

NeighbourGrid::NeighbourGrid(double linearSize, double cellSize) : linearSize{linearSize} {
    Expects(linearSize > 0);
    Expects(cellSize > 0);

    // 2 cells on both edges - "reflected" cells - are used by periodic boundary conditions
    this->numOfRealCells = static_cast<std::size_t>(std::floor(linearSize / cellSize)) + 2;
    ExpectsMsg(this->numOfRealCells >= 3, "Neighbour grid cell too big");
    this->cellSize = this->linearSize / static_cast<double>(this->numOfRealCells - 2);
    auto numCells = static_cast<std::size_t>(std::round(std::pow(this->numOfRealCells, 3)));
    this->cells.resize(numCells);
    this->reflectedCell.resize(numCells, -1);

    // Aliasing "reflected" cell lists to real ones
    for (std::size_t i{}; i < this->cells.size(); i++)
        if (this->isCellReflected(i))
            this->reflectedCell[i] = this->getReflectedCellVector(i);

    this->fillNeighbouringCellsOffsets();
}

void NeighbourGrid::add(std::size_t idx, const Vector<3> &position) {
    int i = this->positionToCellNo(position);
    this->getCellVector(i).push_back(idx);
}

void NeighbourGrid::remove(std::size_t idx, const Vector<3> &position) {
    int i = this->positionToCellNo(position);
    auto &cell = this->getCellVector(i);
    auto it = std::find(cell.begin(), cell.end(), idx);
    if (it != cell.end())
        cell.erase(it);
}

void NeighbourGrid::clear() {
    for (auto &cell : this->cells)
        cell.clear();
}

const std::vector<std::size_t> &NeighbourGrid::getCell(const Vector<3> &position) const {
    int i = this->positionToCellNo(position);
    return this->getCellVector(i);
}

std::vector<std::size_t> NeighbourGrid::getNeighbours(const Vector<3> &position) const {
    std::vector<std::size_t> result;

    int cellNo = this->positionToCellNo(position);
    for (int cellOffset : this->neighbouringCellsOffsets) {
        const auto &cellVector = this->getCellVector(cellNo + cellOffset);
        result.insert(result.end(), cellVector.begin(), cellVector.end());
    }
    return result;
}

std::vector<std::size_t> &NeighbourGrid::getCellVector(std::size_t cellNo) {
    if (this->reflectedCell[cellNo] == -1)
        return this->cells[cellNo];
    else
        return this->cells[this->reflectedCell[cellNo]];
}

const std::vector<std::size_t> &NeighbourGrid::getCellVector(std::size_t cellNo) const {
    if (this->reflectedCell[cellNo] == -1)
        return this->cells[cellNo];
    else
        return this->cells[this->reflectedCell[cellNo]];
}
