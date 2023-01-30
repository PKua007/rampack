//
// Created by Michał Cieśla on 07/03/2017.
// Modified by Piotr Kubala on 26/12/2020.
//

#ifndef RAMPACK_NEIGHBOURGRID_H
#define RAMPACK_NEIGHBOURGRID_H

// Uncomment to sanitize race conditions on cell, i.e. two or more threads altering a single NG cell.
// Cells automatically recognize which thread altered them for the first time after resetting the sanitizer, and they
// block access by different threads.

// #define NG_SANITIZE_RACE_CONDITION

#include <vector>
#include <algorithm>
#include <array>
#include <iterator>
#include <memory>

#include "TriclinicBox.h"
#include "geometry/Vector.h"
#include "utils/Exceptions.h"

/**
 * @brief An acceleration structure for a constant-time lookup of neighbours for a fixed number of particles.
 */
class NeighbourGrid {
private:
    static constexpr std::size_t LIST_END = std::numeric_limits<std::size_t>::max();

    TriclinicBox box;
    std::array<Vector<3>, 3> boxSides;
    std::array<std::size_t, 3> cellDivisions{};
    std::array<double, 3> relativeCellSize{};
    std::vector<std::size_t> cellHeads;
    std::vector<std::size_t> cellOwningThreads;
    std::vector<std::size_t> successors;
    std::array<Vector<3>, 27> translations;
    std::vector<std::size_t> translationIndices;
    std::vector<std::size_t> reflectedCells;
    std::size_t numCells{};
    std::vector<std::size_t> neighbouringCellsOffsets;
    std::vector<std::size_t> positiveNeighbouringCellsOffsets;

    static bool increment(std::array<int, 3> &in);
    static std::size_t flattenTranslationIndex(std::size_t i, std::size_t j, std::size_t k) { return i*3*3 + j*3 + k; }

    [[nodiscard]] std::array<std::size_t, 3> cellNoToCoordinates(std::size_t cellNo) const;
    [[nodiscard]] std::size_t coordinatesToCellNo(const std::array<std::size_t, 3> &coords) const;
    [[nodiscard]] std::size_t realCoordinatesToCellNo(const std::array<std::size_t, 3> &coords) const;
    [[nodiscard]] std::size_t cellNeighbourToCellNo(const std::array<std::size_t, 3> &coords,
                                                    const std::array<int, 3> &neighbour) const;
    [[nodiscard]] std::array<std::pair<double, double>, 3>
    cellCoordinatesToCellBounds(const std::array<std::size_t, 3> &coords) const;

    /**
     * @brief Returns true if @a cellNo is the reflection of a real cell due to periodic boundary conditions
     */
    [[nodiscard]] bool isCellReflected(std::size_t cellNo) const;

    /**
     * @brief If @a cellNo is the reflection of a real cell due to periodic boundary conditions the method returns
     * pointer to the vector in the real cell. Otherwise @a nullptr is returned.
     */
    [[nodiscard]] std::pair<std::size_t, std::size_t> getReflectedCellData(std::size_t cellNo) const;

    void fillNeighbouringCellsOffsets();

    [[nodiscard]] std::vector<std::size_t> getCellVector(std::size_t cellNo) const;
    void setupSizes(const TriclinicBox& newBox, double newCellSize);
    void calculateTranslations();

    void sanitizeRaceCondition(size_t cellNo, const std::string& methodSignature);

    friend class NeighboursView;
    friend class NeighboursViewIterator;
    friend class NeighbourCellData;

public:
    /**
     * @brief Iterator over the cell linked-list.
     */
    class CellViewIterator {
    private:
        const NeighbourGrid *grid;
        std::size_t head;

    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using pointer = void;
        using reference = std::size_t;

        CellViewIterator(const NeighbourGrid *grid, std::size_t head) : grid{grid}, head{head} { }

        CellViewIterator& operator++() {
            this->head = this->grid->successors[this->head];
            return *this;
        }

        CellViewIterator operator++(int) {
            CellViewIterator retval = *this;
            ++(*this);
            return retval;
        }

        bool operator==(CellViewIterator other) const { return this->head == other.head; }
        bool operator!=(CellViewIterator other) const { return !(*this == other);}
        value_type operator*() const { return this->head; }
    };

    /**
     * @brief The view over a single cell linked-list.
     */
    class CellView {
    private:
        const NeighbourGrid &grid;
        std::size_t head;

    public:
        CellView(const NeighbourGrid &grid, std::size_t head) : grid{grid}, head{head} { }

        [[nodiscard]] CellViewIterator begin() const { return CellViewIterator(&this->grid, this->head); }
        [[nodiscard]] CellViewIterator end() const { return CellViewIterator(&this->grid, LIST_END); }
    };


    /**
     * @brief Helper class for NeighboursViewIterator containing points in cell and its BC translation
     */
    class NeighbourCellData {
    private:
        const NeighbourGrid *grid;
        const std::size_t head;
        const Vector<3> *translation;

    public:
        NeighbourCellData(std::size_t head, const Vector<3> *translation, const NeighbourGrid *grid)
                : grid{grid}, head{head}, translation{translation}
        { }

        [[nodiscard]] CellView getNeighbours() const {
            return CellView(*this->grid, this->head);
        }
        [[nodiscard]] const Vector<3> &getTranslation() const { return *this->translation; }
    };

    /**
     * @brief An iterator over all (or half) neighbouring cells of a given cell. Dereferencing it returns the vector of
     * identifiers lying in the given cell.
     */
    class NeighboursViewIterator {
    private:
        const NeighbourGrid *grid;
        const std::vector<std::size_t> *offsets;
        std::size_t cellNo{};
        std::size_t offsetIdx{};

    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = NeighbourCellData;
        using difference_type = std::ptrdiff_t;
        using pointer = void;
        using reference = NeighbourCellData;

        NeighboursViewIterator(const NeighbourGrid *grid, std::size_t cellNo, std::size_t offset,
                               const std::vector<std::size_t> *offsets)
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

        value_type operator*() const {
            std::size_t neighbourCellNo = this->cellNo + (*this->offsets)[this->offsetIdx];
            std::size_t translationIdx = grid->translationIndices[neighbourCellNo];

            return NeighbourCellData(this->grid->cellHeads[this->grid->reflectedCells[neighbourCellNo]],
                                     &(this->grid->translations[translationIdx]), this->grid);
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
            return NeighboursViewIterator(&this->grid, this->cellNo, 0, &this->offsets);
        }

        [[nodiscard]] NeighboursViewIterator end() const {
            return NeighboursViewIterator(&this->grid, this->cellNo, this->numOffsets, &this->offsets);
        }
    };

    /**
     * @brief Creates a neighbour grid for a cubic box of side length @a linearSize. The minimal cell size is given by
     * @a cellSize and the number of supported particles is given by @a numParticles.
     */
    NeighbourGrid(double linearSize, double cellSize, std::size_t numParticles);

    /**
     * @brief Creates a neighbour grid for a cuboidal box of side lengths @a linearSizes. The minimal cell size is
     * given by @a cellSize and the number of supported particles is given by @a numParticles.
     */
    NeighbourGrid(const std::array<double, 3> &linearSizes, double cellSize, std::size_t numParticles);

    /**
     * @brief Creates a neighbour grid for a general box. The minimal cell size is given by @a cellSize and the number
     * of supported particles is given by @a numParticles.
     */
    NeighbourGrid(const TriclinicBox& box, double cellSize, std::size_t numParticles);

    /**
     * @brief Adds an object with identifier @a idx at position @a position to the neighbour grid.
     */
    void add(std::size_t idx, const Vector<3> &position);

    /**
     * @brief Translates @a position to internal cell identifier, which may be later used in
     * NeighbourGrid::add(std::size_t, std::size_t)
     */
    [[nodiscard]] std::size_t positionToCellNo(const Vector<3> &position) const;

    /**
     * @brief Adds an object with identifier @a idx at a cell indexed by @a cellNo.
     */
    void add(std::size_t idx, std::size_t cellNo);

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
     * @brief Resizes the neighbour grid with given new box and new cell size. NG is also cleared.
     */
    bool resize(TriclinicBox box_, double newCellSize);

    /**
     * @brief Returns all identifiers of objects places in NG cell containing @a position point.
     */
    [[nodiscard]] CellView getCell(const Vector<3> &position) const;

    /**
     * @brief Returns all identifiers of objects places in NG cell given by integer coordinates @a coord.
     */
    [[nodiscard]] CellView getCell(const std::array<std::size_t, 3> &coord) const;

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
     * @param onlyPositive if true, only half of the cells will be enumerated (useful for iterating over distinct pairs
     * of NG cells)
     */
    [[nodiscard]] NeighboursView getNeighbouringCells(const Vector<3> &position, bool onlyPositive = false) const;

    /**
     * @brief Returns NeighboursView of all neighbouring cells of the NG cell given by integer coordinates @a coord.
     * @param coord integer coordinates of neighbour grid cell
     * @param onlyPositive if true, only half of the cells will be enumerated (useful for iterating over distinct pairs
     * of NG cells)
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

    /**
     * @brief Resets race condition sanitizer, i.e. all threads lose the "ownership" of cells.
     */
    void resetRaceConditionSanitizer();
};

#endif //RAMPACK_NEIGHBOURGRID_H
