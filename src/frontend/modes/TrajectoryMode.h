//
// Created by Piotr Kubala on 25/01/2023.
//

#ifndef RAMPACK_TRAJECTORYMODE_H
#define RAMPACK_TRAJECTORYMODE_H

#include "frontend/ModeBase.h"
#include "core/Simulation.h"
#include "frontend/RampackParameters.h"


class TrajectoryMode : public ModeBase {
private:
    Simulation::Environment recreateRawEnvironment(const RampackParameters &params, std::size_t startRunIndex) const;

public:
    explicit TrajectoryMode(Logger &logger) : ModeBase(logger) { }

    int main(int argc, char **argv);
};


#endif //RAMPACK_TRAJECTORYMODE_H
