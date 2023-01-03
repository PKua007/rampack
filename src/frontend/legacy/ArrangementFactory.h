//
// Created by pkua on 01.12.2021.
//

#ifndef RAMPACK_ARRANGEMENTFACTORY_H
#define RAMPACK_ARRANGEMENTFACTORY_H

#include <vector>
#include <sstream>
#include <array>
#include <memory>

#include "core/BoundaryConditions.h"
#include "core/Interaction.h"
#include "core/ShapeGeometry.h"
#include "core/Packing.h"


namespace legacy {
    class ArrangementFactory {
    public:
        static std::unique_ptr<Packing> arrangePacking(std::size_t numOfParticles, const std::string &boxString,
                                                       const std::string &arrangementString,
                                                       std::unique_ptr<BoundaryConditions> bc,
                                                       const Interaction &interaction, const ShapeGeometry &geometry,
                                                       std::size_t moveThreads, std::size_t scalingThreads);
    };
}


#endif //RAMPACK_ARRANGEMENTFACTORY_H
