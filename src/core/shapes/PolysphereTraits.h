//
// Created by Piotr Kubala on 22/12/2020.
//

#ifndef RAMPACK_POLYSPHERETRAITS_H
#define RAMPACK_POLYSPHERETRAITS_H

#include <utility>
#include <ostream>
#include <map>
#include <optional>

#include "core/ShapeTraits.h"
#include "interactions/CentralInteraction.h"
#include "OptionalAxis.h"
#include "GenericShapeRegistry.h"


/**
 * @brief Class representing a single species of a polysphere for PolysphereTraits, conforming to GenericShapeRegistry
 * @a ConcreteSpecies template parameter.
 */
class PolysphereShape /* : public GenericShapeRegistry::ConcreteSpecies */ {
public:
    /**
     * @brief A helper class describing a single spherical bead.
     */
    struct SphereData {
        /** @brief Position of sphere's center. */
        const Vector<3> position;
        /** @brief Sphere's radius. */
        const double radius{};

        SphereData(const Vector<3> &position, double radius);

        /**
         * @brief Returns the sphere's center calculated for position and orientation of @a shape.
         */
        [[nodiscard]] Vector<3> centreForShape(const Shape &shape) const;

        /**
         * @brief Returns Wolfram Mathematica representation of the sphere calculated for position and orientation of
         * @a shape.
         */
        void toWolfram(std::ostream &out, const Shape &shape) const;

        friend bool operator==(const SphereData &lhs, const SphereData &rhs) {
            return std::tie(lhs.position, lhs.radius) == std::tie(rhs.position, rhs.radius);
        }

        friend std::ostream &operator<<(std::ostream &os, const SphereData &data) {
            return os << "{" << data.position << ", " << data.radius << "}";
        }
    };

private:
    std::vector<SphereData> sphereData{};
    std::optional<Vector<3>> primaryAxis;
    std::optional<Vector<3>> secondaryAxis;
    Vector<3> geometricOrigin;
    double volume{};
    std::map<std::string, Vector<3>> namedPoints;

    [[nodiscard]] double calculateVolume() const;

public:
    /**
     * @brief Constructs the polysphere with given parameters.
     * @details Apart from the named points passed in the @a customNamedPoints argument, named point for sphere centers
     * are created automatically. Their names are `s[index]`, where `[index]` is the (0-based) index of the sphere in
     * @a sphereData vector.
     * @param sphereData vector of SphereData defining the polysphere
     * @param primaryAxis primary axis of the shape (may be left undefined)
     * @param secondaryAxis secondary axis of the shape, orthogonal to the primary axis (may be left undefined)
     * @param geometricOrigin geometric origin of the shape
     * @param volume volume of the shape. If `std::nullopt` is passed, the volume will be calculated automatically, but
     * only if the spheres in @a sphereData do not overlap. If they do overlap, the volume must be computed manually by
     * the caller and passed here
     * @param customNamedPoints optional map of name points, where the key is point's name and the value is its position
     */
    explicit PolysphereShape(std::vector<SphereData> sphereData, OptionalAxis primaryAxis = std::nullopt,
                             OptionalAxis secondaryAxis = std::nullopt,
                             const Vector<3> &geometricOrigin = {0, 0, 0},
                             std::optional<double> volume = std::nullopt,
                             const std::map<std::string, Vector<3>> &customNamedPoints = {});

    [[nodiscard]] Vector<3> getPrimaryAxis() const /* override */;
    [[nodiscard]] Vector<3> getSecondaryAxis() const /* override */;
    [[nodiscard]] Vector<3> getGeometricOrigin() const /* override */ { return this->geometricOrigin; }
    [[nodiscard]] double getVolume() const /* override */ { return this->volume; }
    [[nodiscard]] const std::map<std::string, Vector<3>> &getNamedPoints() const  /* override */ {
        return this->namedPoints;
    }

    [[nodiscard]] const std::vector<SphereData> &getSphereData() const { return this->sphereData; }
    [[nodiscard]] std::vector<Vector<3>> getInteractionCentres() const;

    /**
     * @brief Calculates mass centre and moves it to {0, 0, 0} (geometric origin and named points are translated
     * accordingly).
     * @details Sphere overlaps are not accounted for.
     */
    void normalizeMassCentre();

    /**
     * @brief Calculates and returns the mass centre.
     * @details Sphere overlaps are not accounted for.
     */
    [[nodiscard]] Vector<3> calculateMassCentre() const;

    void setGeometricOrigin(const Vector<3> &geometricOrigin_) { this->geometricOrigin = geometricOrigin_; }
    void addCustomNamedPoints(std::map<std::string, Vector<3>> customNamedPoints);

    /**
     * @brief Returns @a true is the constituent beads overlap, @a false otherwise.
     */
    [[nodiscard]] bool spheresOverlap() const;

    friend bool operator==(const PolysphereShape &lhs, const PolysphereShape &rhs);
};


/**
 * @brief A polymer consisting of identical or different hard of soft-interacting spheres.
 */
class PolysphereTraits : public ShapeTraits, public GenericShapeRegistry<PolysphereShape> {
private:
    class HardInteraction : public Interaction {
    private:
        const PolysphereTraits &traits;

    public:
        explicit HardInteraction(const PolysphereTraits &traits) : traits{traits} { }

        [[nodiscard]] bool hasHardPart() const override { return true; }
        [[nodiscard]] bool hasSoftPart() const override { return false; }
        [[nodiscard]] bool hasWallPart() const override { return true; }
        [[nodiscard]] bool isConvex() const override { return false; }
        [[nodiscard]] bool overlapBetween(const Vector<3> &pos1, const Matrix<3, 3> &orientation1,
                                          const std::byte *data1, std::size_t idx1, const Vector<3> &pos2,
                                          const Matrix<3, 3> &orientation2, const std::byte *data2, std::size_t idx2,
                                          const BoundaryConditions &bc) const override;
        [[nodiscard]] bool overlapWithWall(const Vector<3> &pos, const Matrix<3, 3> &orientation,
                                           const std::byte *data, std::size_t idx, const Vector<3> &wallOrigin,
                                           const Vector<3> &wallVector) const override;

        [[nodiscard]] std::vector<Vector<3>> getInteractionCentres(const std::byte *data) const override;

        [[nodiscard]] double getRangeRadius(const std::byte *data) const override;
    };

    class WolframPrinter : public ShapePrinter {
    private:
        const PolysphereTraits &traits;

    public:
        explicit WolframPrinter(const PolysphereTraits &traits) : traits{traits} { }

        [[nodiscard]] std::string print(const Shape &shape) const override;
    };

    [[nodiscard]] std::shared_ptr<ShapePrinter> createObjPrinter(std::size_t subdivisions) const;

    std::shared_ptr<Interaction> interaction;
    std::shared_ptr<CentralInteraction> centralInteraction;
    std::shared_ptr<WolframPrinter> wolframPrinter;

    friend HardInteraction;
    friend WolframPrinter;

public:
    using SphereData = PolysphereShape::SphereData;

    /**
     * @brief The default number of sphere subdivisions when printing the shape (see XCPrinter::buildPolyhedron
     * @a subdivisions parameter)
     */
    static constexpr std::size_t DEFAULT_MESH_SUBDIVISIONS = 3;

    /**
     * @brief Creates the class with hard-core interactions and no initially registered species.
     */
    PolysphereTraits();

    /**
     * @brief Creates the class with hard-core interactions and one initial species @a defaultSpecies named `A`, which
     * is set as a default species (setDefaultSpecies()).
     */
    explicit PolysphereTraits(const PolysphereShape &defaultSpecies);

    /**
     * @brief Creates the class with soft interactions @a centralInteraction and no initially registered species.
     */
    explicit PolysphereTraits(const std::shared_ptr<CentralInteraction> &centralInteraction);

    /**
     * @brief Creates the class with soft interactions @a centralInteraction and one species @a shape named `A`, which
     * is set as a default species (setDefaultSpecies()).
     */
    PolysphereTraits(const PolysphereShape &polysphereShape,
                     const std::shared_ptr<CentralInteraction> &centralInteraction);

    PolysphereTraits(const PolysphereTraits &) = delete;
    PolysphereTraits &operator=(const PolysphereTraits &) = delete;
    ~PolysphereTraits() override;

    [[nodiscard]] const Interaction &getInteraction() const override { return *this->interaction; }
    [[nodiscard]] const ShapeGeometry &getGeometry() const override { return *this; }
    [[nodiscard]] const ShapeDataManager &getDataManager() const override { return *this; }

    /**
     * @brief Returns ShapePrinter for a given @a format.
     * @details The following formats are supported:
     * <ol>
     *     <li> `wolfram` - Wolfram Mathematica shape
     *     <li> `obj` - Wavefront OBJ triangle mesh (it accepts @a mesh_divisions parameter, default: 3)
     * </ol>
     */
    [[nodiscard]] std::shared_ptr<const ShapePrinter>
    getPrinter(const std::string &format, const std::map<std::string, std::string> &params) const override;
};


#endif //RAMPACK_POLYSPHERETRAITS_H
