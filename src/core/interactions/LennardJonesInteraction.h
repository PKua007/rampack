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
    [[nodiscard]] double calculateEnergyForDistance2(double distance2) const override;

public:
    LennardJonesInteraction(double epsilon, double sigma);

    [[nodiscard]] double getRangeRadius() const override { return 3 * this->sigma; }
};


#endif //RAMPACK_LENNARDJONESINTERACTION_H
