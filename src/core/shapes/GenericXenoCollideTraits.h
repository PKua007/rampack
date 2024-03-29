//
// Created by pkua on 05.10.22.
//

#ifndef RAMPACK_GENERICXENOCOLLIDETRAITS_H
#define RAMPACK_GENERICXENOCOLLIDETRAITS_H

#include <optional>
#include <map>

#include "XenoCollideTraits.h"
#include "geometry/xenocollide/AbstractXCGeometry.h"


/**
 * @brief XenoCollideTraits using AbstractXCGeometry as @a CollideGeometry.
 * @details It is an adapter class, which enables one to used some implementation of AbstractXCGeometry, for example
 * coming from XCBodyBuilder.
 */
class GenericXenoCollideTraits : public XenoCollideTraits<GenericXenoCollideTraits> {
private:
    std::shared_ptr<AbstractXCGeometry> geometry;

public:
    GenericXenoCollideTraits(std::shared_ptr<AbstractXCGeometry> geometry, OptionalAxis primaryAxis,
                             OptionalAxis secondaryAxis, const Vector<3> &geometricOrigin, double volume,
                             const ShapeGeometry::NamedPoints &namedPoints)
            : XenoCollideTraits(primaryAxis, secondaryAxis, geometricOrigin, volume, namedPoints),
              geometry{std::move(geometry)}
    { }

    [[nodiscard]] const AbstractXCGeometry &getCollideGeometry([[maybe_unused]] std::size_t i = 0) const {
        return *this->geometry;
    }
};


#endif //RAMPACK_GENERICXENOCOLLIDETRAITS_H
