//
// Created by Piotr Kubala on 19/12/2022.
//

#ifndef RAMPACK_OPTIONALAXIS_H
#define RAMPACK_OPTIONALAXIS_H

#include <optional>

#include "geometry/Vector.h"


class OptionalAxis : public std::optional<Vector<3>> {
public:
    using std::optional<Vector<3>>::optional;

    OptionalAxis(std::initializer_list<double> vector) : std::optional<Vector<3>>(Vector<3>(vector)) { }
    OptionalAxis(std::optional<Vector<3>> vector) : std::optional<Vector<3>>(vector) { }
};


#endif //RAMPACK_OPTIONALAXIS_H
