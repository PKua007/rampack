//
// Created by pkua on 12.09.22.
//

#ifndef RAMPACK_BULKOBSERVABLE_H
#define RAMPACK_BULKOBSERVABLE_H

#include "Packing.h"
#include "ShapeTraits.h"


/**
 * @brief An observable which contains many values (for example a whole plot), which can be averaged over many system
 * snapshots.
 */
class BulkObservable {
public:
    virtual ~BulkObservable() = default;

    /**
     * @brief Adds another snapshot to averaging.
     * @param packing snapshot packing
     * @param temperature temperature of the snapshot
     * @param pressure pressure of the snapshot
     * @param shapeTraits ShapeTraits used in the @a packing
     */
    virtual void addSnapshot(const Packing &packing, double temperature, double pressure,
                             const ShapeTraits &shapeTraits) = 0;

    /**
     * @brief Prints the observable onto given output @a out in an observable-specific way.
     */
    virtual void print(std::ostream &out) const = 0;

    /**
     * @brief Cleans all accumulated snapshots.
     */
    virtual void clear() = 0;

    /**
     * @brief Returns a (short) name of this observable for filenames and reporting.
     */
    [[nodiscard]] virtual std::string getSignatureName() const = 0;
};


#endif //RAMPACK_BULKOBSERVABLE_H
