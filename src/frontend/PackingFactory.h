//
// Created by Piotr Kubala on 16/01/2023.
//

#ifndef RAMPACK_PACKINGFACTORY_H
#define RAMPACK_PACKINGFACTORY_H

#include <memory>

#include "core/Packing.h"
#include "core/ShapeTraits.h"


class PackingFactory {
public:
    virtual ~PackingFactory() = default;
    virtual std::unique_ptr<Packing> createPacking(std::unique_ptr<BoundaryConditions> bc,
                                                   const ShapeTraits &shapeTraits, std::size_t moveThreads,
                                                   std::size_t scalingThreads) = 0;
};


#endif //RAMPACK_PACKINGFACTORY_H
