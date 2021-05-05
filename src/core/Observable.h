//
// Created by Piotr Kubala on 22/03/2021.
//

#ifndef RAMPACK_OBSERVABLE_H
#define RAMPACK_OBSERVABLE_H

#include <vector>
#include <string>

#include "Packing.h"
#include "ShapeTraits.h"

class Observable {
public:
    virtual ~Observable() = default;

    virtual void calculate(const Packing &packing, double temperature, double pressure,
                           const ShapeTraits &shapeTraits) = 0;
    [[nodiscard]] virtual std::vector<std::string> getIntervalHeader() const = 0;
    [[nodiscard]] virtual std::vector<std::string> getNominalHeader() const = 0;
    [[nodiscard]] virtual std::vector<double> getIntervalValues() const = 0;
    [[nodiscard]] virtual std::vector<std::string> getNominalValues() const = 0;
    [[nodiscard]] virtual std::string getName() const = 0;
};


#endif //RAMPACK_OBSERVABLE_H
