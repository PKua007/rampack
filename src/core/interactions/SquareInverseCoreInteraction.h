//
// Created by pkua on 05.03.2022.
//

#ifndef RAMPACK_SQUAREINVERSECOREINTERACTION_H
#define RAMPACK_SQUAREINVERSECOREINTERACTION_H

#include "CentralInteraction.h"


class SquareInverseCoreInteraction : public CentralInteraction {
private:
    double epsilon{};
    double sigma{};

protected:
    [[nodiscard]] double calculateEnergyForDistance2(double distance2) const override;

public:
    SquareInverseCoreInteraction(double epsilon, double sigma);

    [[nodiscard]] double getRangeRadius() const override { return this->sigma; }
};


#endif //RAMPACK_SQUAREINVERSECOREINTERACTION_H
