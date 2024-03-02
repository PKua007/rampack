//
// Created by ciesla on 12/04/23.
//

#include "PolyhedralWedgeTraits.h"
#include "utils/Exceptions.h"
#include "geometry/xenocollide/XCBodyBuilder.h"
#include "core/io/ShapeDataSerializer.h"
#include "core/io/ShapeDataDeserializer.h"


PolyhedralWedgeTraits::CollideGeometry::CollideGeometry(double bottomAx, double bottomAy, double topAx, double topAy,
                                                        double l)
        : vertexUp{topAx/2, topAy/2, l/2}, vertexDown{bottomAx/2, bottomAy/2, -l/2}
{
    Expects((topAx > 0 && bottomAy > 0) || (topAy > 0 && bottomAx > 0));
    Expects(l > 0);

    this->circumsphereRadius = std::max(this->vertexUp.norm(), this->vertexDown.norm());

    double irUp = std::min(topAx, topAy);
    double irDown = std::min(bottomAx, bottomAy);
    this->insphereRadius = std::min(std::min(irUp, irDown), l) / 2;
}


double PolyhedralWedgeShape::computeVolume(double bottomAx, double bottomAy, double topAx, double topAy, double l) {
    return l/6*(2*bottomAx*bottomAy + topAx*bottomAy + bottomAx*topAy + 2*topAx*topAy);
}

PolyhedralWedgeShape::PolyhedralWedgeShape(double bottomAx, double bottomAy, double topAx, double topAy, double l,
                                           std::size_t subdivisions)
        : bottomAx{bottomAx}, bottomAy{bottomAy}, topAx{topAx}, topAy{topAy}, l{l},
          volume{PolyhedralWedgeShape::computeVolume(bottomAx, bottomAy, topAx, topAy, l)},
          begNamedPoint{0, 0, -l/2}, endNamedPoint{0, 0, l/2}
{
    Expects((topAx > 0 && bottomAy > 0) || (topAy > 0 && bottomAx > 0));
    Expects(l > 0);

    this->subdivisions = subdivisions == 0 ? 1 : subdivisions;
    if (this->subdivisions == 1) {
        this->interactionCentres = {};
        this->shapeParts.emplace_back(this->bottomAx, this->bottomAy, this->topAx, this->topAy, this->l);
        return;
    }

    double dl = this->l / static_cast<double>(subdivisions);
    double dy = (this->bottomAy - this->topAy) / static_cast<double>(subdivisions);
    double dx = (this->bottomAx - this->topAx) / static_cast<double>(subdivisions);
    this->shapeParts.reserve(subdivisions);
    this->interactionCentres.reserve(subdivisions);
    for (std::size_t i{}; i < subdivisions; i++) {
        double subAxBottom = this->topAx + static_cast<double>(i + 1) * dx;
        double subAyBottom = this->topAy + static_cast<double>(i + 1) * dy;
        double subAxTop = this->topAx + static_cast<double>(i) * dx;
        double subAyTop = this->topAy + static_cast<double>(i) * dy;

        this->shapeParts.emplace_back(subAxBottom, subAyBottom, subAxTop, subAyTop, dl);
        this->interactionCentres.push_back({0, 0, l / 2.0 - (static_cast<double>(i) + 0.5) * dl});
    }
}

bool PolyhedralWedgeShape::equal(double bottomAx_, double bottomAy_, double topAx_, double topAy_, double l_,
                                 std::size_t subdivisions_) const
{
    return std::tie(this->bottomAx, this->bottomAy, this->topAx, this->topAy, this->l, this->subdivisions)
        == std::tie(bottomAx_, bottomAy_, topAx_, topAy_, l_, subdivisions_);
}

PolyhedralWedgeTraits::PolyhedralWedgeTraits(std::optional<double> defaultBottomAx,
                                             std::optional<double> defaultBottomAy,
                                             std::optional<double> defaultTopAx, std::optional<double> defaultTopAy,
                                             std::optional<double> defaultL, std::size_t defaultSubdivisions)
{
    if (defaultBottomAx)
        Expects(defaultBottomAx >= 0);
    if (defaultBottomAy)
        Expects(defaultBottomAy >= 0);
    if (defaultTopAx)
        Expects(defaultTopAx >= 0);
    if (defaultTopAy)
        Expects(defaultTopAy >= 0);
    if (defaultL)
        Expects(defaultL > 0);
    if (defaultBottomAx && defaultBottomAy && defaultTopAx && defaultTopAy)
        Expects((*defaultTopAx > 0 && *defaultBottomAy > 0) || (*defaultTopAy > 0 && *defaultBottomAx > 0));

    ShapeDataSerializer serializer;
    if (defaultBottomAx)
        serializer["bottom_ax"] = *defaultBottomAx;
    if (defaultBottomAy)
        serializer["bottom_ay"] = *defaultBottomAy;
    if (defaultTopAx)
        serializer["top_ax"] = *defaultTopAx;
    if (defaultTopAy)
        serializer["top_ay"] = *defaultTopAy;
    if (defaultL)
        serializer["l"] = *defaultL;
    serializer["subdivisions"] = defaultSubdivisions;
    this->setDefaultShapeData(serializer.toTextualShapeData());

    this->registerDynamicNamedPoint("beg", [this](const ShapeData &data) {
        return this->speciesFor(data).getBegNamedPoint();
    });
    this->registerDynamicNamedPoint("end", [this](const ShapeData &data) {
        return this->speciesFor(data).getEndNamedPoint();
    });
}

std::shared_ptr<const ShapePrinter>
PolyhedralWedgeTraits::getPrinter(const std::string &format, const std::map<std::string, std::string> &params) const {
    // We override the function from XenoCollideTraits not to redundantly print subdivisions

    std::size_t meshSubdivisions = XenoCollideTraits::DEFAULT_MESH_SUBDIVISIONS;
    if (params.find("mesh_divisions") != params.end()) {
        meshSubdivisions = std::stoul(params.at("mesh_divisions"));
        Expects(meshSubdivisions >= 1);
    }

    if (format == "wolfram")
        return this->createPrinter<PolydisperseXCWolframShapePrinter>(meshSubdivisions);
    else if (format == "obj")
        return this->createPrinter<PolydisperseXCObjShapePrinter>(meshSubdivisions);
    else
        throw NoSuchShapePrinterException("XenoCollideTraits: unknown printer format: " + format);
}

TextualShapeData PolyhedralWedgeTraits::serialize(const ShapeData &data) const {
    const auto &shape = speciesFor(data);
    ShapeDataSerializer serializer;
    serializer["bottom_ax"] = shape.getBottomAx();
    serializer["bottom_ay"] = shape.getBottomAy();
    serializer["top_ax"] = shape.getTopAx();
    serializer["top_ay"] = shape.getTopAy();
    serializer["l"] = shape.getL();
    serializer["subdivisions"] = shape.getSubdivisions();
    return serializer.toTextualShapeData();
}

ShapeData PolyhedralWedgeTraits::deserialize(const TextualShapeData &data) const {
    ShapeDataDeserializer deserializer(data);
    auto bottomAx = deserializer.as<double>("bottom_ax");
    auto bottomAy = deserializer.as<double>("bottom_ay");
    auto topAx = deserializer.as<double>("top_ax");
    auto topAy = deserializer.as<double>("top_ay");
    auto l = deserializer.as<double>("l");
    auto subdivisions = deserializer.as<std::size_t>("subdivisions");
    deserializer.throwIfNotAccessed();

    ShapeDataValidateMsg(bottomAx >= 0, "Bottom x side must be >= 0");
    ShapeDataValidateMsg(bottomAy >= 0, "Bottom y side must be >= 0");
    ShapeDataValidateMsg(topAx >= 0, "Top x side must be >= 0");
    ShapeDataValidateMsg(topAy >= 0, "Top y side must be >= 0");
    ShapeDataValidateMsg((topAx > 0 && bottomAy > 0) || (topAy > 0 &&bottomAx > 0),
                         "Given side dimensions cannot yield a flat shape");
    ShapeDataValidateMsg(l > 0, "Length must be > 0");
    if (subdivisions == 0)
        subdivisions = 1;
    return this->shapeDataForSpecies(bottomAx, bottomAy, topAx, topAy, l, subdivisions);
}

ShapeData PolyhedralWedgeTraits::shapeDataForSpecies(double bottomAx, double bottomAy, double topAx, double topAy, double l,
                                                     std::size_t subdivisions) const
{
    if (subdivisions == 0)
        subdivisions = 1;

    return this->shapeDataForSpeciesImpl(bottomAx, bottomAy, topAx, topAy, l, subdivisions);
}