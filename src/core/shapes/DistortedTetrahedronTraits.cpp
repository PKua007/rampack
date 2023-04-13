//
// Created by ciesla on 12/04/23.
//

#include "DistortedTetrahedronTraits.h"
#include "utils/Exceptions.h"
#include "geometry/xenocollide/XCBodyBuilder.h"


double DistortedTetrahedronTraits::getVolume(double R, double r, double l) {
    return (1.0/12.0)*R*r*l;
}

DistortedTetrahedronTraits::DistortedTetrahedronTraits(double R, double r, double l)
        : XenoCollideTraits({0, 0, 1}, {1, 0, 0}, {0, 0, 0},
                            DistortedTetrahedronTraits::getVolume(R, r, l),
                            {{"beg", {0, 0, -l/2}}, {"end", {0, 0, l/2}}}), R{R},
          r{r}, l{l}, shapeModel(R, r, l)
{
    Expects(r > 0);
    Expects(R > 0);
    Expects(l > 0);
}

std::shared_ptr<const ShapePrinter>
DistortedTetrahedronTraits::getPrinter(const std::string &format, const std::map<std::string, std::string> &params) const {
    // We override the function from XenoCollideTraits not to redundantly print subdivisions

    std::size_t meshSubdivisions = XenoCollideTraits::DEFAULT_MESH_SUBDIVISIONS;
    if (params.find("mesh_divisions") != params.end()) {
        meshSubdivisions = std::stoul(params.at("mesh_divisions"));
        Expects(meshSubdivisions >= 1);
    }

    if (format == "wolfram")
        return this->createPrinter<XCWolframShapePrinter>(meshSubdivisions);
    else if (format == "obj")
        return this->createPrinter<XCObjShapePrinter>(meshSubdivisions);
    else
        throw NoSuchShapePrinterException("XenoCollideTraits: unknown printer format: " + format);
}

DistortedTetrahedronTraits::CollideGeometry::CollideGeometry(double R, double r, double l)
        : R{R}, r{r}, l{l},
        circumsphereRadius{std::sqrt(std::max(R, r)*std::max(R, r) + l*l/4)},
        insphereRadius{std::min(std::min(R, r), l)/2.0}
{
    Expects(R > 0);
    Expects(r > 0);
    Expects(l > 0);
}
