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
#include "core/shapes/XCGeometryCenter.h"


/**
 * @brief Base class for all XenoCollide ShapePrinter -s with the support for polydispersity.
 * @details <p> It support shapes with both a single AbstractXCGeometry for a single interaction center as well as a set
 * of multiple AbstractXCGeometry sub-parts (which is called PolydisperseXCShapePrinter::GeometryComplex) for multiple
 * interaction centers. The class is responsible to generate triangle meshes for all geometries (called
 * PolydisperseXCShapePrinter::PolyhedronComplex), while the deriving class is only responsible for printing the
 * triangle mesh in a particular output format (see doPrint()). For the performance reason,
 * PolydisperseXCShapePrinter::PolyhedronComplex instances generated for given ShapeData are cached and do not need to
 * be recreated each time.
 *
 * <p> The mesh is created based on the XenoCollide support function. See XCPrinter for the details on how it is done.
 * The number of iterations is given by @a subdivisions parameter of the constructor.
 */
class PolydisperseXCShapePrinter : public ShapePrinter {
public:
    /**
     * @brief A set of XCGeometryCenter -s (AbstractXCGeometry plus interaction center positions) constituting a
     * compound shape.
     */
    using GeometryComplex = std::vector<XCGeometryCenter>;

    /**
     * @brief Functor providing a pointer to singular AbstractXCGeometry for a given ShapeData.
     */
    using GeometryProvider = std::function<std::shared_ptr<const AbstractXCGeometry>(const ShapeData &)>;

    /**
     * @brief Functor providing a whole GeometryComplex for a given ShapeData.
     */
    using GeometryComplexProvider = std::function<GeometryComplex(const ShapeData &)>;

protected:
    /**
     * @brief A set of many Polyhedron instances constituting a compound shape triangle mesh.
     */
    using PolyhedronComplex = std::vector<Polyhedron>;

    /**
     * @brief Converts a given PolyhedronComplex @a polyhedronComplex (placed and orientation as the Shape @a shape) to
     * an implementation-specific textual form and returns it.
     */
    [[nodiscard]] virtual std::string doPrint(const Shape &shape, const PolyhedronComplex &polyhedronComplex) const = 0;

private:
    [[nodiscard]] PolyhedronComplex buildPolyhedronComplex(const GeometryComplex &geometryComplex) const;
    [[nodiscard]] const PolyhedronComplex &findPolyhedronComplex(const ShapeData &data) const;

    GeometryComplexProvider geometryComplexProvider;
    std::size_t subdivisions{};
    mutable std::vector<std::pair<ShapeData, PolyhedronComplex>> polyhedronComplexCache;

public:
    /**
     * @brief Constructs the class for a single AbstractXCGeometry (for a shape with a single interaction center).
     * @param geometryProvider the functor providing AbstractXCGeometry for given ShapeData
     * @param subdivisions the number of mesh refining iterations
     */
    PolydisperseXCShapePrinter(const GeometryProvider &geometryProvider, std::size_t subdivisions)
            : geometryComplexProvider{[geometryProvider](const ShapeData &data) -> GeometryComplex{
                  return {XCGeometryCenter{geometryProvider(data)}};
              }},
              subdivisions{subdivisions}
    { }

    /**
     * @brief Constructs the class for a PolydisperseXCShapePrinter::GeometryComplex (for a shape with multiple
     * interaction centers).
     * @param geometryComplexProvider the functor providing GeometryComplex for given ShapeData
     * @param subdivisions the number of mesh refining iterations
     */
    PolydisperseXCShapePrinter(GeometryComplexProvider geometryComplexProvider, std::size_t subdivisions)
            : geometryComplexProvider{std::move(geometryComplexProvider)}, subdivisions{subdivisions}
    { }

    [[nodiscard]] std::string print(const Shape &shape) const final;
};


#endif //RAMPACK_POLYDISPERSEXCSHAPEPRINTER_H
