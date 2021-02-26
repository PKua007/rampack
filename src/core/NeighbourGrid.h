//
// Created by Michał Cieśla on 07/03/2017.
// Modified by Piotr Kubala on 26/12/2020.
//

#ifndef RAMPACK_NEIGHBOURGRID_H
#define RAMPACK_NEIGHBOURGRID_H

#include <vector>
#include <algorithm>
#include <array>
#include <iterator>

#include "geometry/Vector.h"
#include "utils/Assertions.h"

class NeighbourGrid {
private:
    std::array<double, 3> linearSize{};
    std::array<std::size_t, 3> numCellsInLine{};
    std::array<double, 3> cellSize{};
    std::vector<std::vector<std::size_t>> cells;
    std::vector<int> reflectedCells;
    std::size_t numCells{};
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
    [[nodiscard]] int getReflectedCellNo(std::size_t cellNo) const;

    static bool increment(std::array<int, 3> &in);
    void fillNeighbouringCellsOffsets();

    std::vector<std::size_t> &getCellVector(std::size_t cellNo);
    [[nodiscard]] const std::vector<std::size_t> &getCellVector(std::size_t cellNo) const;
    void setupSizes(const std::array<double, 3> &newLinearSize, double newCellSize);

    friend class NeighboursView;
    friend class NeighboursViewIterator;

public:
    class NeighboursViewIterator : public std::iterator<std::input_iterator_tag, std::vector<std::size_t>,
                                                        std::ptrdiff_t,
                                                        const std::vector<std::size_t>*,
                                                        const std::vector<std::size_t> &>
    {
    private:
        const NeighbourGrid &grid;
        std::size_t cellNo{};
        std::size_t offset{};

    public:
        NeighboursViewIterator(const NeighbourGrid &grid, std::size_t cellNo, std::size_t offset)
                : grid{grid}, cellNo{cellNo}, offset{offset}
        { }

        NeighboursViewIterator& operator++() {
            this->offset++;
            return *this;
        }

        NeighboursViewIterator operator++(int) {
            NeighboursViewIterator retval = *this;
            ++(*this);
            return retval;
        }

        bool operator==(NeighboursViewIterator other) const {
            return this->offset == other.offset;
        }

        bool operator!=(NeighboursViewIterator other) const {
            return !(*this == other);
        }

        reference operator*() const {
            return this->grid.getCellVector(this->cellNo + this->grid.neighbouringCellsOffsets[this->offset]);
        }
    };

    class NeighboursView {
    private:
        const NeighbourGrid &grid;
        std::size_t cellNo{};
        std::size_t numOfOffsets{};

    public:
        explicit NeighboursView(const NeighbourGrid &grid, std::size_t cellNo)
                : grid{grid}, cellNo{cellNo}, numOfOffsets{grid.neighbouringCellsOffsets.size()}
        { }

        [[nodiscard]] NeighboursViewIterator begin() const {
            return NeighboursViewIterator(this->grid, this->cellNo, 0);
        }

        [[nodiscard]] NeighboursViewIterator end() const {
            return NeighboursViewIterator(this->grid, this->cellNo, this->numOfOffsets);
        }
    };

    NeighbourGrid(double linearSize, double cellSize);
    NeighbourGrid(const std::array<double, 3> &linearSize, double cellSize);

    void add(std::size_t idx, const Vector<3> &position);
    void remove(std::size_t idx, const Vector<3> &position);
    void clear();
    void resize(double newLinearSize, double newCellSize);
    void resize(const std::array<double, 3> &newLinearSize, double newCellSize);
    [[nodiscard]] const std::vector<std::size_t> &getCell(const Vector<3> &position) const;
    [[nodiscard]] std::vector<std::size_t> getNeighbours(const Vector<3> &position) const;
    [[nodiscard]] NeighboursView getNeighbouringCells(const Vector<3> &position) const;

    friend void swap(NeighbourGrid &ng1, NeighbourGrid &ng2);
};

#endif //RAMPACK_NEIGHBOURGRID_H
