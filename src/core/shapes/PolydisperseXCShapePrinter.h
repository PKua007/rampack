//
// Created by Piotr Kubala on 29/02/2024.
//

#ifndef RAMPACK_POLYDISPERSEXCSHAPEPRINTER_H
#define RAMPACK_POLYDISPERSEXCSHAPEPRINTER_H

#include <utility>
#include <functional>

#include "core/ShapePrinter.h"
#include "geometry/xenocollide/AbstractXCGeometry.h"
#include "geometry/Polyhedron.h"


class PolydisperseXCShapePrinter : public ShapePrinter {
public:
    struct GeometryData {
        Vector<3> center;
        std::shared_ptr<const AbstractXCGeometry> geometry;
    };

    using GeometryProvider = std::function<std::vector<GeometryData>(const ShapeData &)>;

protected:
    struct PolyhedronData {
        Vector<3> center;
        Polyhedron polyhedron;

        PolyhedronData(const Vector<3> &center, Polyhedron polyhedron)
                : center{center}, polyhedron{std::move(polyhedron)}
        { }
    };

    using PolyhedronComplex = std::vector<PolyhedronData>;

    [[nodiscard]] virtual std::string doPrint(const Shape &shape, const PolyhedronComplex &polyhedronComplex) const = 0;

private:
    [[nodiscard]] PolyhedronComplex buildPolyhedronComplex(const std::vector<GeometryData> &geometries) const;
    [[nodiscard]] const PolyhedronComplex &findPolyhedronComplex(const ShapeData &data) const;

    GeometryProvider geometryProvider;
    std::size_t subdivisions{};
    mutable std::vector<std::pair<ShapeData, PolyhedronComplex>> polyhedronCache;


public:
    PolydisperseXCShapePrinter(GeometryProvider geometryProvider, std::size_t subdivisions)
            : geometryProvider{std::move(geometryProvider)}, subdivisions{subdivisions}
    { }

    [[nodiscard]] std::string print(const Shape &shape) const final;
};


#endif //RAMPACK_POLYDISPERSEXCSHAPEPRINTER_H
