//
// Created by pkua on 02.02.2022.
//

#ifndef RAMPACK_BOX_H
#define RAMPACK_BOX_H

#include <vector>

#include "Shape.h"
#include "geometry/Vector.h"


class Box {
public:
    virtual ~Box() = default;

    virtual void absoluteToRelative(std::vector<Shape> &shapes) const = 0;
    [[nodiscard]] virtual Vector<3> absoluteToRelative(const Vector<3> &pos) const = 0;
    virtual void relativeToAbsolute(std::vector<Shape> &shapes) const = 0;
    [[nodiscard]] virtual Vector<3> relativeToAbsolute(const Vector<3> &pos) const = 0;
};


#endif //RAMPACK_BOX_H
