//
// Created by pkua on 29.05.22.
//

#ifndef RAMPACK_LATTICEBUILDER_H
#define RAMPACK_LATTICEBUILDER_H

#include <string>

#include "core/Packing.h"
#include "core/ShapeTraits.h"
#include "core/lattice/Lattice.h"


namespace legacy {
    class LatticeBuilder {
    public:
        static std::vector<std::string> getSupportedCellTypes();
        static std::unique_ptr<Packing> buildPacking(std::size_t numParticles, const std::string &boxString,
                                                     const std::string &arrangementString,
                                                     std::unique_ptr<BoundaryConditions> bc,
                                                     const ShapeTraits &shapeTraits, std::size_t moveThreads,
                                                     std::size_t scalingThreads);
    };
}


#endif //RAMPACK_LATTICEBUILDER_H
