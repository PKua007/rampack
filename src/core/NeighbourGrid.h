//
// Created by Michał Cieśla on 07/03/2017.
// Modified by Piotr Kubala on 26/12/2020.
//

#ifndef RAMPACK_NEIGHBOURGRID_H
#define RAMPACK_NEIGHBOURGRID_H

#include <vector>
#include <algorithm>
#include <array>

#include "geometry/Vector.h"
#include "utils/Assertions.h"

class NeighbourGrid {
private:
    double linearSize;
    std::size_t numOfRealCells;
    double cellSize;

    std::vector<std::vector<std::size_t> *> cells;
    std::vector<long> neighbouringCellsOffsets;

    [[nodiscard]] std::size_t positionToCellNo(const Vector<3> &position) const {
        std::size_t result = 0;
        std::size_t ix;
        for (int i = 2; i >= 0; i--) {
            Expects(position[i] >= 0);
            Expects(position[i] < this->linearSize);

            // +1, since first row of cells on each edges is "reflected", not "real"
            ix = static_cast<int>(position[i]/this->cellSize) + 1;
            result = this->numOfRealCells * result + ix;
        }
        return result;
    }

    [[nodiscard]] std::array<std::size_t, 3> cellNoToCoordinates(std::size_t cellNo) const {
        std::array<std::size_t, 3> result{};
        for (std::size_t i{}; i < 3; i++) {
            result[i] = cellNo % this->numOfRealCells;
            cellNo /= this->numOfRealCells;
        }
        return result;
    }

    [[nodiscard]] std::size_t coordinatesToCellNo(const std::array<std::size_t, 3> &coords) const {
        std::size_t result{};
        for (int i = 2; i >= 0; i--)
            result = this->numOfRealCells * result + coords[i];
        return result;
    }

    [[nodiscard]] std::size_t cellNeighbourToCellNo(const std::array<std::size_t, 3> &coordinates,
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

    /*
     * @brief Returns true if @a cellNo is the reflection of a real cell due to periodic boundary conditions
     */
    [[nodiscard]] bool isReflectedCell(std::size_t cellNo) const {
        std::array<std::size_t, 3> coords = this->cellNoToCoordinates(cellNo);
        return std::any_of(coords.begin(), coords.end(), [this](auto coord) {
            return coord == 0 || coord == this->numOfRealCells - 1;
        });
    }

    /*
     * @brief If @a cellNo is the reflection of a real cell due to periodic boundary conditions the method returns
     * pointer to the vector in the real cell. Otherwise @a nullptr is returned.
     */
    [[nodiscard]] std::vector<std::size_t> *getReflectedCellVector(std::size_t cellNo) const {
        if (!this->isReflectedCell(cellNo))
            return nullptr;

        std::array<std::size_t, 3> coords = this->cellNoToCoordinates(cellNo);
        for (auto &coord : coords) {
            if (coord == 0)
                coord = this->numOfRealCells - 2;
            if (coord == this->numOfRealCells - 1)
                coord = 1;
        }
        return this->cells[this->coordinatesToCellNo(coords)];
    }

    static bool increment(std::array<int, 3> &in) {
        for (std::size_t i{}; i < 3; i++) {
            in[i]++;
            if (in[i] > 2 && i < 2)
                in[i] = 0;
            else
                break;
        }
        return in[2] <= 2;
    }

    void fillNeigbouringCellsOffsets() {
        // 3 x 3 neighbours
        this->neighbouringCellsOffsets.reserve(27);

        // We are taking the cell somewhere in the middle and computing offsets in cell list to all of its neighbours
        std::array<int, 3> in{};
        std::array<std::size_t, 3> coords{};
        in.fill(0);
        coords.fill(this->numOfRealCells / 2);
        std::size_t testCellNo = this->coordinatesToCellNo(coords);
        do {
            std::size_t neigbourNo = this->cellNeighbourToCellNo(coords, in);
            this->neighbouringCellsOffsets.push_back(neigbourNo - testCellNo);
        } while(increment(in));

        // sort and erase to avoid duplicates - important for small packings
        std::sort( this->neighbouringCellsOffsets.begin(), this->neighbouringCellsOffsets.end());
        this->neighbouringCellsOffsets.erase(std::unique(this->neighbouringCellsOffsets.begin(),
                                                         this->neighbouringCellsOffsets.end()),
                                             this->neighbouringCellsOffsets.end());
    }

public:
    NeighbourGrid(double linearSize, double cellSize) : linearSize{linearSize} {
        Expects(linearSize > 0);
        Expects(cellSize > 0);

        // 2 cells on both edges are used by periodic boundary conditions
        this->numOfRealCells = static_cast<std::size_t>(std::floor(linearSize / cellSize)) + 2;
        ExpectsMsg(this->numOfRealCells >= 3, "Neighbour grid cell too big");
        this->cellSize = this->linearSize / static_cast<double>(this->numOfRealCells - 2);
        auto numCells = static_cast<std::size_t>(std::round(std::pow(this->numOfRealCells, 3)));
        this->cells = std::vector<std::vector<std::size_t> *>(numCells);

        // Allocating real cell lists
        for (std::size_t i{}; i < numCells; i++) {
            if (this->isReflectedCell(i))
                continue;
            this->cells[i] = new std::vector<std::size_t>;
        }

        // Aliasing reflected cell lists to real ones
        for (std::size_t i{}; i < numCells; i++)
            if (this->isReflectedCell(i))
                this->cells[i] = this->getReflectedCellVector(i);

        // Sanity check - all cells should have lists
        for (std::size_t i{}; i < numCells; i++)
            Assert(this->cells[i] != nullptr);

        this->fillNeigbouringCellsOffsets();
    }

    NeighbourGrid(const NeighbourGrid &other) = delete;
    NeighbourGrid &operator=(const NeighbourGrid &other) = delete;

    ~NeighbourGrid() {
        for (std::size_t i{}; i < this->cells.size(); i++)
            if (!this->isReflectedCell(i))
                delete this->cells[i];
    }

    void add(std::size_t idx, const Vector<3> &position) {
        int i = this->positionToCellNo(position);
        this->cells[i]->push_back(idx);
    }

    void remove(std::size_t idx, const Vector<3> &position) {
        int i = this->positionToCellNo(position);
        auto it = std::find(this->cells[i]->begin(), this->cells[i]->end(), idx);
        if (it != this->cells[i]->end())
            this->cells[i]->erase(it);
    }

    void clear() {
        for (auto cell : this->cells)
            cell->clear();
    }

    [[nodiscard]] const std::vector<std::size_t> &getCell(const Vector<3> &position) const {
        int i = this->positionToCellNo(position);
        return *(this->cells[i]);
    }

    [[nodiscard]] std::vector<std::size_t> getNeighbours(const Vector<3> &position) const {
        std::vector<std::size_t> result;

        int cellNo = this->positionToCellNo(position);
        for (int cellOffset : this->neighbouringCellsOffsets) {
            auto cellVector = this->cells[cellNo + cellOffset];
            result.insert(result.end(), cellVector->begin(), cellVector->end());
        }
        return result;
    }
};


#endif //RAMPACK_NEIGHBOURGRID_H
