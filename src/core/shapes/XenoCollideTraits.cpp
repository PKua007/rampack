//
// Created by ciesla on 8/9/22.
//

#include "XenoCollideTraits.h"
#include "geometry/xenocollide/Collide.h"
#include "geometry/xenocollide/CollideGeometry.h"
#include "geometry/xenocollide/BodyBuilder.h"


XenoCollideTraits::XenoCollideTraits(Vector<3> pa, Vector<3> sa, Vector<3> cm, double v, const std::string &attr,
                                     std::map<std::string, Vector<3>> customNamedPoints)
        : primaryAxis{pa}, secondaryAxis{sa}, geometricOrigin{cm}, volume{v},
          customNamedPoints{std::move(customNamedPoints)}
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
                                     MapPtr<CollideGeometry> shapeModel, double rangeRadius,
                                     std::map<std::string, Vector<3>> customNamedPoints)
        : primaryAxis{pa}, secondaryAxis{sa}, geometricOrigin{cm}, volume{v}, rangeRadius{rangeRadius},
          customNamedPoints{std::move(customNamedPoints)}, shapeModel{shapeModel}
{

}

bool XenoCollideTraits::overlapBetween(const Vector<3> &pos1, const Matrix<3, 3> &orientation1,
                                       [[maybe_unused]] std::size_t idx1, const Vector<3> &pos2,
                                       const Matrix<3, 3> &orientation2, [[maybe_unused]] std::size_t idx2,
                                       const BoundaryConditions &bc) const
{
    Vector<3> pos2bc = pos2 + bc.getTranslation(pos1, pos2);
    bool result = Collide::Intersect(*(this->shapeModel), orientation1, pos1, *(this->shapeModel), orientation2, pos2bc, 1.0e-12);
    return result;
}

bool XenoCollideTraits::overlapWithWall(const Vector<3> &pos, const Matrix<3, 3> &orientation, [[maybe_unused]] std::size_t idx,
                                   const Vector<3> &wallOrigin, const Vector<3> &wallVector) const{

    Vector<3> normalVector = (orientation.transpose())*(wallVector);
    Vector<3> sp = (*(this->shapeModel)).GetSupportPoint(-normalVector);
    Vector<3> origin = (orientation.transpose())*(wallOrigin - pos);
    double distanceSupport = -sp*normalVector;  // minus sign because we count distance along -normalVector
    double distanceWall = -origin*normalVector;
    if (distanceWall > distanceSupport)
        return false;
    return true;
}


Vector<3> XenoCollideTraits::getNamedPoint(const std::string &pointName, const Shape &shape) const {
    auto namedPoint = this->customNamedPoints.find(pointName);
    if (namedPoint == this->customNamedPoints.end())
        return ShapeGeometry::getNamedPoint(pointName, shape);

    return shape.getPosition() + shape.getOrientation() * namedPoint->second;
}