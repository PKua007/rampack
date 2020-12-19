//
// Created by Piotr Kubala on 19/12/2020.
//

#ifndef RAMPACK_HARDSHAPE_H
#define RAMPACK_HARDSHAPE_H

#include "BoundaryConditions.h"

class HardShape {
public:
    virtual ~HardShape() = default;

    [[nodiscard]] virtual bool overlap(const HardShape &other, double scaleFactor,
                                       const BoundaryConditions &bc) const = 0;
};


#endif //RAMPACK_HARDSHAPE_H
