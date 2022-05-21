//
// Created by pkua on 19.05.22.
//

#include <numeric>
#include <algorithm>

#include "RandomPopulator.h"
#include "utils/Assertions.h"


std::vector<Shape> RandomPopulator::populateLattice(const Lattice &lattice, std::size_t numOfShapes) const {
    std::size_t latticeSize = lattice.size();
    Expects(latticeSize >= numOfShapes);

    auto allShapes = lattice.generateMolecules();
    std::vector<std::size_t> idx(latticeSize, 0);
    std::iota(idx.begin(), idx.end(), 0);
    std::shuffle(idx.begin(), idx.end(), this->rng);
    idx.erase(idx.begin() + static_cast<std::ptrdiff_t>(numOfShapes), idx.end());
    std::sort(idx.begin(), idx.end());

    std::vector<Shape> shapes;
    shapes.reserve(numOfShapes);
    for (auto i : idx)
        shapes.push_back(allShapes[i]);

    return shapes;
}
