//
// Created by ciesla on 12/04/23.
//

#include "PolyhedralWedgeTraits.h"
#include "utils/Exceptions.h"
#include "geometry/xenocollide/XCBodyBuilder.h"


double PolyhedralWedgeTraits::getVolume(double axBottom, double ayBottom, double axTop, double ayTop, double length) {
    double v = length / 6 * (2 * axBottom * ayBottom + axTop * ayBottom + axBottom * ayTop + 2 * axTop * ayTop);
    return v;
}

/*PolyhedralWedgeTraits::PolyhedralWedgeTraits(double axBottom, double ayBottom, double axTop, double ayTop,
                                             double length,
                                             std::size_t subdivisions)
        : XenoCollideTraits({0, 0, 1}, {1, 0, 0}, {0, 0, 0},
                            PolyhedralWedgeTraits::getVolume(axBottom, ayBottom, axTop, ayTop, length),
                            {{"beg", {0, 0, -length / 2}}, {"end", {0, 0, length / 2}}}),
          axBottom{axBottom}, ayBottom{ayBottom}, axTop{axTop}, ayTop{ayTop}, length{length}
{
    Expects((axTop > 0 && ayBottom > 0) || (ayTop > 0 && axBottom > 0));
    Expects(length > 0);

    if (subdivisions == 0 || subdivisions == 1) {
        this->interactionCentres = {};
        this->shapeModels.emplace_back(this->axBottom, this->ayBottom, this->axTop, this->ayTop, this->length);
        return;
    }

    double dl = this->length / static_cast<double>(subdivisions);
    double dy = (this->ayBottom - this->ayTop) / static_cast<double>(subdivisions);
    double dx = (this->axBottom - this->axTop) / static_cast<double>(subdivisions);
    this->shapeModels.reserve(subdivisions);
    this->interactionCentres.reserve(subdivisions);
    for (std::size_t i{}; i < subdivisions; i++) {
        double subAxBottom = this->axTop + static_cast<double>(i + 1) * dx;
        double subAyBottom = this->ayTop + static_cast<double>(i + 1) * dy;
        double subAxTop = this->axTop + static_cast<double>(i) * dx;
        double subAyTop = this->ayTop + static_cast<double>(i) * dy;

        this->shapeModels.emplace_back(subAxBottom, subAyBottom, subAxTop, subAyTop, dl);
        this->interactionCentres.push_back({0, 0, length / 2.0 - (static_cast<double>(i) + 0.5) * dl});
    }
}*/

/*std::shared_ptr<const ShapePrinter>
PolyhedralWedgeTraits::getPrinter(const std::string &format, const std::map<std::string, std::string> &params) const {
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
}*/

PolyhedralWedgeTraits::CollideGeometry::CollideGeometry(double axBottom, double ayBottom, double axTop, double ayTop,
                                                        double length)
        : vertexUp{axTop / 2, ayTop / 2, length / 2}, vertexDown{axBottom / 2, ayBottom / 2, -length / 2}
{
    Expects((axTop > 0 && ayBottom > 0) || (ayTop > 0 && axBottom > 0));
    Expects(length > 0);

    this->circumsphereRadius = std::max(this->vertexUp.norm(), this->vertexDown.norm());

    double irUp = std::min(axTop, ayTop);
    double irDown = std::min(axBottom, ayBottom);
    this->insphereRadius = std::min(std::min(irUp, irDown), length) / 2;
}
