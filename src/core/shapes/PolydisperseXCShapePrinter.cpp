//
// Created by Piotr Kubala on 29/02/2024.
//

#include "PolydisperseXCShapePrinter.h"
#include "geometry/xenocollide/XCPrinter.h"


PolydisperseXCShapePrinter::PolyhedronComplex
PolydisperseXCShapePrinter::buildPolyhedronComplex(const std::vector<GeometryData> &geometries) const {
    PolyhedronComplex polyhedronComplex;
    polyhedronComplex.reserve(geometries.size());
    for (const auto &[center, geometry] : geometries) {
        auto polyhedron = XCPrinter::buildPolyhedron(*geometry, this->subdivisions);
        polyhedronComplex.push_back(polyhedron.transformed(center, Matrix<3, 3>::identity()));
    }
    return polyhedronComplex;
}

std::string PolydisperseXCShapePrinter::print(const Shape &shape) const {
    const auto &shapeData = shape.getData();
    const auto &polyhedronComplex = this->findPolyhedronComplex(shapeData);
    return this->doPrint(shape, polyhedronComplex);
}

const PolydisperseXCShapePrinter::PolyhedronComplex &PolydisperseXCShapePrinter::findPolyhedronComplex(const ShapeData &data) const {
    for (const auto &[shapeData, polyhedronComplex] : this->polyhedronComplexCache)
        if (shapeData == data)
            return polyhedronComplex;

    PolyhedronComplex polyhedronComplex = this->buildPolyhedronComplex(this->geometryComplexProvider(data));
    this->polyhedronComplexCache.emplace_back(data, std::move(polyhedronComplex));
    return this->polyhedronComplexCache.back().second;
}
