//
// Created by ciesla on 8/9/22.
//

#include "RoundedConeTraits.h"
#include "utils/Assertions.h"
#include "geometry/xenocollide/BodyBuilder.h"

MapPtr<CollideGeometry> RoundedConeTraits::createShapeModel(double R, double r, double l) {
    Expects(R > 0);
    Expects(r > 0);
    Expects(l > 0);
    Expects(R >= r);

    BodyBuilder bb;
    bb.sphere(R);
    bb.move(0, 0, -l/2);
    bb.sphere(r);
    bb.move(0, 0, l/2);
    bb.wrap();
    return bb.getCollideGeometry();
}

double RoundedConeTraits::getVolume(double R, double r, double l) {
    return
            (M_PI*
             (l*l*l*l*(r*r + r*R + R*R) +
              2*l*l*(r - R)*(r - R)*(r*r + r*R + R*R) -
              (r - R)*(r - R)*(r - R)*(r - R)*(r*r + r*R + R*R) +
              2*l*l*l*(r*r*r + R*R*R))
            )/(3*l*l*l);
}

RoundedConeTraits::RoundedConeTraits(double R, double r, double l)
        : XenoCollideTraits({0, 0, 1}, {1, 0, 0}, {0, 0, 0},
                            RoundedConeTraits::getVolume(R, r, l),
                            RoundedConeTraits::createShapeModel(R, r, l),
                            l + 2*std::max(1., r),
                            {{"sl", {0, 0, -l/2}}, {"ss", {0, 0, l/2}}}),
          R{R}, r{r}, l{l}
{

}

std::string RoundedConeTraits::toWolfram(const Shape &shape) const {
    Vector<3> pos = shape.getPosition();
    Matrix<3, 3> orientation = shape.getOrientation();

    int ballsNo = 10;
    std::stringstream out;
    out << std::fixed;
    out << "GeometricTransformation[" << std::endl;
    out << "    {";
    for(int i=0; i<ballsNo; i++){
        out << "Sphere[{0, 0, "<< -l/2 + l*(((double)i)/(ballsNo-1)) << "}, " << ((ballsNo-1.0-i)*R + i*r)/(ballsNo-1) <<"]";
        if (i<ballsNo-1)
            out <<", ";
    }
    out << "}, " << std::endl;
    out << "    AffineTransform[" << std::endl;
    out << "        {{{" << orientation(0, 0) << ", " << orientation(0, 1) << ", " << orientation(0, 2) << "}," << std::endl;
    out << "          {" << orientation(1, 0) << ", " << orientation(1, 1) << ", " << orientation(1, 2) << "}," << std::endl;
    out << "          {" << orientation(2, 0) << ", " << orientation(2, 1) << ", " << orientation(2, 2) << "}}," << std::endl;
    out << "          " << pos << "}]]";
    return out.str();
}
