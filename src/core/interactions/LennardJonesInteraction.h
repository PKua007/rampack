//
// Created by Piotr Kubala on 22/12/2020.
//

#ifndef RAMPACK_LENNARDJONESINTERACTION_H
#define RAMPACK_LENNARDJONESINTERACTION_H

#include "CentralInteraction.h"

class LennardJonesInteraction : public CentralInteraction {
private:
    double epsilon{};
    double sigma{};

protected:
    [[nodiscard]] double calculateEnergyForDistance(double distance) const override;

public:
    LennardJonesInteraction(double epsilon, double sigma);
};


#endif //RAMPACK_LENNARDJONESINTERACTION_H
