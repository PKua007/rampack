//
// Created by ciesla on 8/9/22.
//

#include "XenoCollideTraits.h"
#include "geometry/xenocollide/Quat.h"
#include "geometry/xenocollide/Collide.h"
#include "geometry/xenocollide/CollideGeometry.h"
#include "geometry/xenocollide/BodyBuilder.h"


XenoCollideTraits::XenoCollideTraits(Vector<3> pa, Vector<3> sa, Vector<3> cm, double v, const std::string &attr)
        : primaryAxis{pa}, secondaryAxis{sa}, geometricOrigin{cm}, volume{v}
{
    std::stringstream ss(attr);
    std::string commands;
    std::getline(ss, commands, '\0');
    std::string script = "script " + commands;

    BodyBuilder bb;
    bb.ProcessCommand(script);
    this->shapeModel = bb.getCollideGeometry();
    this->rangeRadius = 2*bb.getMaxRadius();
}

XenoCollideTraits::XenoCollideTraits(Vector<3> pa, Vector<3> sa, Vector<3> cm, double v,
                                     MapPtr<CollideGeometry> shapeModel, double rangeRadius)
        : primaryAxis{pa}, secondaryAxis{sa}, geometricOrigin{cm}, volume{v}, rangeRadius{rangeRadius},
          shapeModel{shapeModel}
{

}

bool XenoCollideTraits::overlapBetween(const Vector<3> &pos1, const Matrix<3, 3> &orientation1,
                                       [[maybe_unused]] std::size_t idx1, const Vector<3> &pos2,
                                       const Matrix<3, 3> &orientation2, [[maybe_unused]] std::size_t idx2,
                                       const BoundaryConditions &bc) const
{
    Vector<3> pos2bc = pos2 + bc.getTranslation(pos1, pos2);
    Quat q1(orientation1);
    Quat q2(orientation2);
    bool result = Collide::Intersect(*(this->shapeModel), q1, pos1, *(this->shapeModel), q2, pos2bc, 1.0e-12);
    return result;
}
