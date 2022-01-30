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

/**
 * @brief An acceleration structure for a constant-time lookup of neighbours.
 */
class NeighbourGrid {
private:
    std::array<double, 3> linearSize{};
    std::array<std::size_t, 3> cellDivisions{};
    std::array<double, 3> cellSize{};
    std::vector<std::vector<std::size_t>> cells;
    std::vector<Vector<3>> translations;
    std::vector<std::size_t> reflectedCells;
    std::size_t numCells{};
    std::vector<std::size_t> neighbouringCellsOffsets;
    std::vector<std::size_t> positiveNeighbouringCellsOffsets;

    [[nodiscard]] std::size_t positionToCellNo(const Vector<3> &position) const;
    [[nodiscard]] std::array<std::size_t, 3> cellNoToCoordinates(std::size_t cellNo) const;
    [[nodiscard]] std::size_t coordinatesToCellNo(const std::array<std::size_t, 3> &coords) const;
    [[nodiscard]] std::size_t realCoordinatesToCellNo(const std::array<std::size_t, 3> &coords) const;
    [[nodiscard]] std::size_t cellNeighbourToCellNo(const std::array<std::size_t, 3> &coords,
                                                    const std::array<int, 3> &neighbour) const;

    /**
     * @brief Returns true if @a cellNo is the reflection of a real cell due to periodic boundary conditions
     */
    [[nodiscard]] bool isCellReflected(std::size_t cellNo) const;

    /**
     * @brief If @a cellNo is the reflection of a real cell due to periodic boundary conditions the method returns
     * pointer to the vector in the real cell. Otherwise @a nullptr is returned.
     */
    [[nodiscard]] std::pair<std::size_t, Vector<3>> getReflectedCellData(std::size_t cellNo) const;

    static bool increment(std::array<int, 3> &in);
    void fillNeighbouringCellsOffsets();

    std::vector<std::size_t> &getCellVector(std::size_t cellNo);
    [[nodiscard]] const std::vector<std::size_t> &getCellVector(std::size_t cellNo) const;
    void setupSizes(const std::array<double, 3> &newLinearSize, double newCellSize);

    friend class NeighboursView;
    friend class NeighboursViewIterator;

public:
    /**
     * @brief Helper class for NeighboursViewIterator containing points in cell and its BC translation
     */
    class NeighbourCellData {
    private:
        const std::vector<std::size_t> *neighbours;
        const Vector<3> *translation;

    public:
        NeighbourCellData(const std::vector<std::size_t> *neighbours, const Vector<3> *translation)
                : neighbours{neighbours}, translation{translation}
        { }

        [[nodiscard]] const std::vector<std::size_t> &getNeighbours() const { return *this->neighbours; }
        [[nodiscard]] const Vector<3> &getTranslation() const { return *this->translation; }
    };

    /**
     * @brief An iterator over all (or half) neighbouring cells of a given cell. Dereferencing it returns the vector of
     * identifiers lying in the given cell.
     */
    class NeighboursViewIterator : public std::iterator<std::input_iterator_tag,
                                                        std::pair<const std::vector<std::size_t>*, const Vector<3>*>,
                                                        std::ptrdiff_t,
                                                        const NeighbourCellData*,
                                                        const NeighbourCellData>
    {
    private:
        const NeighbourGrid &grid;
        const std::vector<std::size_t> &offsets;
        std::size_t cellNo{};
        std::size_t offsetIdx{};

    public:
        NeighboursViewIterator(const NeighbourGrid &grid, std::size_t cellNo, std::size_t offset,
                               const std::vector<std::size_t> &offsets)
                : grid{grid}, offsets{offsets}, cellNo{cellNo}, offsetIdx{offset}
        { }

        NeighboursViewIterator& operator++() {
            this->offsetIdx++;
            return *this;
        }

        NeighboursViewIterator operator++(int) {
            NeighboursViewIterator retval = *this;
            ++(*this);
            return retval;
        }

        bool operator==(NeighboursViewIterator other) const {
            return this->offsetIdx == other.offsetIdx;
        }

        bool operator!=(NeighboursViewIterator other) const {
            return !(*this == other);
        }

        reference operator*() const {
            std::size_t neighbourCellNo = this->cellNo + this->offsets[this->offsetIdx];

            return NeighbourCellData(&this->grid.cells[this->grid.reflectedCells[neighbourCellNo]],
                                     &this->grid.translations[neighbourCellNo]);
        }
    };

    /**
     * @brief A view of neighbours of the given cell in a neighbour grid.
     */
    class NeighboursView {
    private:
        const NeighbourGrid &grid;
        const std::vector<std::size_t> &offsets;
        std::size_t cellNo{};
        std::size_t numOffsets{};

    public:
        explicit NeighboursView(const NeighbourGrid &grid, std::size_t cellNo, const std::vector<std::size_t> &offsets)
                : grid{grid}, offsets{offsets}, cellNo{cellNo}, numOffsets{offsets.size()}
        { }

        [[nodiscard]] NeighboursViewIterator begin() const {
            return NeighboursViewIterator(this->grid, this->cellNo, 0, this->offsets);
        }

        [[nodiscard]] NeighboursViewIterator end() const {
            return NeighboursViewIterator(this->grid, this->cellNo, this->numOffsets, this->offsets);
        }
    };

    /**
     * @brief Creates a neighbour grid for a cubic box of side length @a linearSize. The minimal cell size is given by
     * @a cellSize.
     */
    NeighbourGrid(double linearSize, double cellSize);

    /**
     * @brief Creates a neighbour grid for a cuboidal box of side lengths @a linearSize The minimal cell size is given
     * by @a cellSize.
     */
    NeighbourGrid(const std::array<double, 3> &linearSize, double cellSize);

    /**
     * @brief Adds an object with identifier @a idx at position @a position to the neighbour grid.
     */
    void add(std::size_t idx, const Vector<3> &position);

    /**
     * @brief Removes an object with identifier @a idx at position @a position from the neighbour grid.
     */
    void remove(std::size_t idx, const Vector<3> &position);

    /**
     * @brief Clears the neighbour grid.
     */
    void clear();

    /**
     * @brief Resizes the neighbour grid with given new linear size (of a cubic box) and new cell size. NG is also
     * cleared.
     */
    bool resize(double newLinearSize, double newCellSize);

    /**
     * @brief Resizes the neighbour grid with given new linear size (of a cuboidal box) and new cell size. NG is also
     * cleared.
     */
    bool resize(const std::array<double, 3> &newLinearSize, double newCellSize);

    /**
     * @brief Returns all identifiers of objects places in NG cell containing @a position point.
     */
    [[nodiscard]] const std::vector<std::size_t> &getCell(const Vector<3> &position) const;

    /**
     * @brief Returns all identifiers of objects places in NG cell given by integer coordinates @a coord.
     */
    [[nodiscard]] const std::vector<std::size_t> &getCell(const std::array<std::size_t, 3> &coord) const;

    /**
     * @brief Returns all identifiers of objects in NG cell containing @a position point and in neighbouring cells.
     * @details As a result, all objects potentially interacting with a one placed in @a position should be listed.
     * The methods dynamically allocates the memory for the list, so NeighbourGrid::getNeighbouringCells method
     * should be used instead for maximal performance.
     */
    [[nodiscard]] std::vector<std::size_t> getNeighbours(const Vector<3> &position) const;

    /**
     * @brief Returns NeighboursView of all neighbouring cells of the NG cell containing @a position point.
     * @param position the position somewhere inside a given NG cell (identifing it)
     * @param if true, only half of the cells will be enumerated (useful for iterating over distinct pairs of NG cells)
     */
    [[nodiscard]] NeighboursView getNeighbouringCells(const Vector<3> &position, bool onlyPositive = false) const;

    /**
     * @brief Returns NeighboursView of all neighbouring cells of the NG cell given by integer coordinates @a coord.
     * @param coord integer coordinates of neighbour grid cell
     * @param if true, only half of the cells will be enumerated (useful for iterating over distinct pairs of NG cells)
     */
    [[nodiscard]] NeighboursView getNeighbouringCells(const std::array<std::size_t, 3> &coord,
                                                      bool onlyPositive = false) const;

    /**
     * @brief Returns a number of NG cells in each direction.
     */
    [[nodiscard]] std::array<std::size_t, 3> getCellDivisions() const;

    /**
     * @brief Estimates the memory usage of the neighbour grid in bytes.
     */
    [[nodiscard]] std::size_t getMemoryUsage() const;

    friend void swap(NeighbourGrid &ng1, NeighbourGrid &ng2);
};

#endif //RAMPACK_NEIGHBOURGRID_H
