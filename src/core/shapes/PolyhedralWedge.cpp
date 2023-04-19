//
// Created by ciesla on 12/04/23.
//

#include "PolyhedralWedge.h"
#include "utils/Exceptions.h"
#include "geometry/xenocollide/XCBodyBuilder.h"


double PolyhedralWedge::getVolume(double rxTop, double ryTop, double rxBottom, double ryBottom, double length) {
    double v = (2 * length / 3) * (2 * rxBottom * ryBottom + rxTop * ryBottom + rxBottom * ryTop + 2 * rxTop * ryTop);
    return v;
}

PolyhedralWedge::PolyhedralWedge(double rxTop, double ryTop, double rxBottom, double ryBottom, double length,
                                 std::size_t subdivisions)
        : XenoCollideTraits({0, 0, 1}, {1, 0, 0}, {0, 0, 0},
                            PolyhedralWedge::getVolume(rxTop, ryTop, rxBottom, ryBottom, length),
                            {{"beg", {0, 0, -length / 2}}, {"end", {0, 0, length / 2}}}),
          rxTop{rxTop}, ryTop{ryTop}, rxBottom{rxBottom}, ryBottom{ryBottom}, length{length}
{
    Expects((rxTop > 0 && ryBottom > 0) || (ryTop > 0 && rxBottom > 0));
    Expects(length > 0);

    if (subdivisions == 0 || subdivisions == 1) {
        this->interactionCentres = {};
        this->shapeModels.emplace_back(this->rxTop, this->ryTop, this->rxBottom, this->ryBottom, this->length);
        return;
    }

    double dl = this->length / static_cast<double>(subdivisions);
    double dy = (this->ryBottom - this->ryTop) / static_cast<double>(subdivisions);
    double dx = (this->rxBottom - this->rxTop) / static_cast<double>(subdivisions);
    this->shapeModels.reserve(subdivisions);
    this->interactionCentres.reserve(subdivisions);
    for (std::size_t i{}; i < subdivisions; i++) {
        double rxUp = this->rxTop + static_cast<double>(i) * dx;
        double ryUp = this->ryTop + static_cast<double>(i) * dy;
        double rxDown = this->rxTop + static_cast<double>(i + 1) * dx;
        double ryDown = this->ryTop + static_cast<double>(i + 1) * dy;

        if (rxUp > std::max(this->rxTop, this->rxBottom)) rxUp = std::max(this->rxTop, this->rxBottom);
        if (rxUp < std::min(this->rxTop, this->rxBottom)) rxUp = std::min(this->rxTop, this->rxBottom);

        if (rxDown > std::max(this->rxTop, this->rxBottom)) rxDown = std::max(this->rxTop, this->rxBottom);
        if (rxDown < std::min(this->rxTop, this->rxBottom)) rxDown = std::min(this->rxTop, this->rxBottom);

        if (ryUp > std::max(this->ryTop, this->ryBottom)) ryUp = std::max(this->ryTop, this->ryBottom);
        if (ryUp < std::min(this->ryTop, this->ryBottom)) ryUp = std::min(this->ryTop, this->ryBottom);

        if (ryDown > std::max(this->ryTop, this->ryBottom)) ryDown = std::max(this->ryTop, this->ryBottom);
        if (ryDown < std::min(this->ryTop, this->ryBottom)) ryDown = std::min(this->ryTop, this->ryBottom);

        this->shapeModels.emplace_back(rxUp, ryUp, rxDown, ryDown, dl);
        this->interactionCentres.push_back({0, 0, length / 2.0 - (static_cast<double>(i) + 0.5) * dl});
    }
}


std::shared_ptr<const ShapePrinter>
PolyhedralWedge::getPrinter(const std::string &format, const std::map<std::string, std::string> &params) const {
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

PolyhedralWedge::CollideGeometry::CollideGeometry(double rxTop, double ryTop, double rxBottom, double ryBottom, double length)
        : rxTop{rxTop}, ryTop{ryTop}, rxBottom{rxBottom}, ryBottom{ryBottom}, length{length}
{
    Expects((rxTop > 0 && ryBottom > 0) || (ryTop > 0 && rxBottom > 0));
    Expects(length > 0);

    double crUp = std::sqrt(rxTop * rxTop + ryTop * ryTop + length * length / 4.0);
    double crDown = std::sqrt(rxBottom * rxBottom + ryBottom * ryBottom + length * length / 4.0);
    this->circumsphereRadius = std::max(crUp, crDown);

    double irUp = std::min(rxTop, ryTop);
    double irDown = std::min(rxBottom, ryBottom);
    this->insphereRadius = std::min(std::min(irUp, irDown), length / 2.0);
}
