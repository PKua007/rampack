//
// Created by ciesla on 8/9/22.
//

#include "SmoothWedgeTraits.h"
#include "utils/Assertions.h"
#include "geometry/xenocollide/XCBodyBuilder.h"


double SmoothWedgeTraits::getVolume(double R, double r, double l) {
    double r2 = r*r;
    double r3 = r2*r;
    double R2 = R*R;
    double R3 = R2*R;
    double Rr = R*r;
    double epsilon = (R - r)/l;
    double epsilon2 = epsilon*epsilon;

    return M_PI*l/3*(R2 + Rr + r2)*(1 + epsilon2) + 2*M_PI/3*(R3 + r3);
}

SmoothWedgeTraits::SmoothWedgeTraits(double R, double r, double l)
        : XenoCollideTraits({0, 0, 1}, {1, 0, 0}, {0, 0, (r - R)/2},
                            SmoothWedgeTraits::getVolume(R, r, l),
                            l + 2*std::max(R, r),
                            {{"sl", {0, 0, -l/2}}, {"ss", {0, 0, l/2}}}),
          R{R}, r{r}, l{l}, shapeModel(R, r, l)
{

}

std::string SmoothWedgeTraits::toWolfram(const Shape &shape) const {
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

SmoothWedgeTraits::CollideGeometry::CollideGeometry(double R, double r, double l)
        : R{R}, r{r}, l{l}, Rminusr{R - r}, Rpos{-l/2}, rpos{l/2}, circumsphereRadius{l/2 + R}
{
    Expects(R > 0);
    Expects(r > 0);
    Expects(l > 0);
    Expects(R >= r);
}
