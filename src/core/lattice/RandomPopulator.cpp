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
    std::vector<std::size_t> moleculeIdxs(latticeSize, 0);
    std::iota(moleculeIdxs.begin(), moleculeIdxs.end(), 0);
    std::shuffle(moleculeIdxs.begin(), moleculeIdxs.end(), this->rng);
    moleculeIdxs.erase(moleculeIdxs.begin() + static_cast<std::ptrdiff_t>(numOfShapes), moleculeIdxs.end());
    std::sort(moleculeIdxs.begin(), moleculeIdxs.end());

    std::vector<Shape> shapes;
    shapes.reserve(numOfShapes);
    for (auto i : moleculeIdxs)
        shapes.push_back(allShapes[i]);

    return shapes;
}
