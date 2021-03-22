//
// Created by Piotr Kubala on 22/03/2021.
//

#ifndef RAMPACK_OBSERVABLE_H
#define RAMPACK_OBSERVABLE_H

#include <vector>
#include <string>

#include "Packing.h"

class Observable {
public:
    virtual ~Observable() = default;

    virtual void calculate(const Packing &packing) = 0;
    virtual std::vector<std::string> getHeader() const = 0;
    virtual std::vector<double> getValues() const = 0;
    virtual std::string getName() const = 0;
};


#endif //RAMPACK_OBSERVABLE_H
