//
// Created by Piotr Kubala on 22/12/2020.
//

#ifndef RAMPACK_REPULSIVELENNARDJONESINTERACTION_H
#define RAMPACK_REPULSIVELENNARDJONESINTERACTION_H

#include "CentralInteraction.h"

class RepulsiveLennardJonesInteraction : public CentralInteraction {
private:
    double epsilon{};
    double sigma{};
    double sigmaTimesTwoToOneSixth{};

protected:
    [[nodiscard]] double calculateEnergyForDistance2(double distance2) const override;

public:
    RepulsiveLennardJonesInteraction(double epsilon, double sigma);

    [[nodiscard]] double getRangeRadius() const override { return this->sigmaTimesTwoToOneSixth; }
};


#endif //RAMPACK_REPULSIVELENNARDJONESINTERACTION_H
