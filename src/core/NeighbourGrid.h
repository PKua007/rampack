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

    [[nodiscard]] std::size_t positionToCellNo(const Vector<3> &position) const;
    [[nodiscard]] std::array<std::size_t, 3> cellNoToCoordinates(std::size_t cellNo) const;
    [[nodiscard]] std::size_t coordinatesToCellNo(const std::array<std::size_t, 3> &coords) const;
    [[nodiscard]] std::size_t cellNeighbourToCellNo(const std::array<std::size_t, 3> &coordinates,
                                                    const std::array<int, 3> &neighbour) const;

    /*
     * @brief Returns true if @a cellNo is the reflection of a real cell due to periodic boundary conditions
     */
    [[nodiscard]] bool isCellReflected(std::size_t cellNo) const;

    /*
     * @brief If @a cellNo is the reflection of a real cell due to periodic boundary conditions the method returns
     * pointer to the vector in the real cell. Otherwise @a nullptr is returned.
     */
    [[nodiscard]] std::vector<std::size_t> *getReflectedCellVector(std::size_t cellNo) const;

    static bool increment(std::array<int, 3> &in);
    void fillNeighbouringCellsOffsets();

public:
    NeighbourGrid(double linearSize, double cellSize);
    NeighbourGrid(const NeighbourGrid &other) = delete;
    NeighbourGrid &operator=(const NeighbourGrid &other) = delete;
    ~NeighbourGrid();

    void add(std::size_t idx, const Vector<3> &position);
    void remove(std::size_t idx, const Vector<3> &position);
    void clear();
    [[nodiscard]] const std::vector<std::size_t> &getCell(const Vector<3> &position) const;
    [[nodiscard]] std::vector<std::size_t> getNeighbours(const Vector<3> &position) const;

    void allocateCellLists();
};


#endif //RAMPACK_NEIGHBOURGRID_H
