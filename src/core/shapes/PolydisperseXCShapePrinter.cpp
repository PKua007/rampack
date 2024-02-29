//
// Created by Piotr Kubala on 29/02/2024.
//

#include "PolydisperseXCShapePrinter.h"
#include "geometry/xenocollide/XCPrinter.h"


PolydisperseXCShapePrinter::PolyhedronComplex
PolydisperseXCShapePrinter::buildPolyhedronComplex(const std::vector<GeometryData> &geometries) const {
    PolyhedronComplex polyhedronComplex;
    polyhedronComplex.reserve(geometries.size());
    for (const auto &[center, geometry] : geometries)
        polyhedronComplex.emplace_back(center, XCPrinter::buildPolyhedron(*geometry, this->subdivisions));
    return polyhedronComplex;
}

std::string PolydisperseXCShapePrinter::print(const Shape &shape) const {
    const auto &shapeData = shape.getData();
    const auto &polyhedronComplex = this->findPolyhedronComplex(shapeData);
    return this->doPrint(shape, polyhedronComplex);
}

const PolydisperseXCShapePrinter::PolyhedronComplex &PolydisperseXCShapePrinter::findPolyhedronComplex(const ShapeData &data) const {
    for (const auto &[shapeData, polyhedronComplex] : this->polyhedronCache)
        if (shapeData == data)
            return polyhedronComplex;

    PolyhedronComplex polyhedronComplex = this->buildPolyhedronComplex(this->geometryComplexProvider(data));
    this->polyhedronCache.emplace_back(data, std::move(polyhedronComplex));
    return this->polyhedronCache.back().second;
}
