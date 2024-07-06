//
// Created by Piotr Kubala on 05/07/2024.
//

#ifndef RAMPACK_XCGEOMETRYCENTER_H
#define RAMPACK_XCGEOMETRYCENTER_H

#include <memory>

#include "geometry/Matrix.h"
#include "geometry/xenocollide/AbstractXCGeometry.h"


/**
* @brief Geometry data of a single interaction center of XenoCollide Interaction.
*/
struct XCGeometryCenter {
    explicit XCGeometryCenter(std::shared_ptr<const AbstractXCGeometry> geometry = {}, const Vector<3> center = {})
            : geometry{std::move(geometry)}, center{center}
    { }

    /** @brief Pointer to XenoCollide geometry of the interaction center. */
    std::shared_ptr<const AbstractXCGeometry> geometry;

    /** @brief Position of the interaction center. */
    Vector<3> center;
};


#endif //RAMPACK_XCGEOMETRYCENTER_H
