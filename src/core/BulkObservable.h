//
// Created by pkua on 12.09.22.
//

#ifndef RAMPACK_BULKOBSERVABLE_H
#define RAMPACK_BULKOBSERVABLE_H

#include "Packing.h"
#include "ShapeTraits.h"


class BulkObservable {
public:
    virtual ~BulkObservable() = default;

    virtual void addSnapshot(const Packing &packing, double temperature, double pressure,
                             const ShapeTraits &shapeTraits) = 0;
    virtual void print(std::ostream &out) const = 0;
    virtual void clear() = 0;
};


#endif //RAMPACK_BULKOBSERVABLE_H
