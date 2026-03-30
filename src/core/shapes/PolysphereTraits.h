//
// Created by Piotr Kubala on 22/12/2020.
//

#ifndef RAMPACK_POLYSPHERETRAITS_H
#define RAMPACK_POLYSPHERETRAITS_H

#include <utility>
#include <ostream>
#include <map>
#include <optional>
#include <vector>

#include "core/ShapeTraits.h"
#include "core/interactions/CentralInteractionBase.h"
#include "core/interactions/InteractionCentreLayout.h"
#include "OptionalAxis.h"


/**
 * @brief A polymer consisting of identical or different hard- or soft-interacting spheres.
 * @details The hard representation is always described bead-by-bead by SphereData. Soft central interactions may,
 * however, use a more general interaction-centre layout with explicit centre types.
 */
class PolysphereTraits : public ShapeTraits {
public:
    /**
     * @brief A helper class describing a single spherical bead.
     */
    struct SphereData {
        const Vector<3> position;
        const double radius{};

        SphereData(const Vector<3> &position, double radius);

        [[nodiscard]] Vector<3> centreForShape(const Shape &shape) const;
        void toWolfram(std::ostream &out, const Shape &shape) const;

        friend bool operator==(const SphereData &lhs, const SphereData &rhs) {
            return std::tie(lhs.position, lhs.radius) == std::tie(rhs.position, rhs.radius);
        }

        friend std::ostream &operator<<(std::ostream &os, const SphereData &data) {
            return os << "{" << data.position << ", " << data.radius << "}";
        }
    };

    /**
     * @brief A helper class storing per-type metadata for interaction centres.
     * @details At the moment it stores only the radius used to reconstruct SphereData and to visualize interaction
     * centres of a given type, but it may be extended in the future with further per-type attributes.
     */
    struct InteractionCentreTypeMetadata {
        double radius{};

        explicit InteractionCentreTypeMetadata(double radius);
    };

    /**
     * @brief A helper class storing interaction-centre layout together with per-type metadata.
     * @details This is the typed backend construction API for PolysphereGeometry. It separates interaction-centre
     * positions and their types from metadata associated with each type.
     */
    class InteractionCentreLayoutWithMetadata {
    private:
        InteractionCentreLayout interactionCentreLayout;
        std::vector<InteractionCentreTypeMetadata> centreTypeMetadata;

    public:
        InteractionCentreLayoutWithMetadata() = default;

        /**
         * @brief Constructs the typed interaction-centre description of a polysphere.
         * @param interactionCentreLayout interaction-centre positions together with centre types
         * @param centreTypeMetadata metadata for all interaction-centre types
         */
        InteractionCentreLayoutWithMetadata(InteractionCentreLayout interactionCentreLayout,
                                            std::vector<InteractionCentreTypeMetadata> centreTypeMetadata);

        /**
         * @brief Returns the interaction-centre layout of the polysphere.
         */
        [[nodiscard]] const InteractionCentreLayout &getInteractionCentreLayout() const {
            return this->interactionCentreLayout;
        }

        /**
         * @brief Returns metadata for all interaction-centre types.
         */
        [[nodiscard]] const std::vector<InteractionCentreTypeMetadata> &getCentreTypeMetadata() const {
            return this->centreTypeMetadata;
        }
    };

    /**
     * @brief A helper class defining the geometry of the particle.
     * @details The class, apart from standard named points (see ShapeGeometry::getNamedPoint()) and
     * @a customNamedPoints from the constructor, defines points "s[x]" representing constituent spheres, where "[x]" is
     * sphere's index starting from 0.
     */
    class PolysphereGeometry : public ShapeGeometry {
    private:
        std::vector<SphereData> sphereData;
        InteractionCentreLayout interactionCentreLayout;
        std::vector<double> displayRadiiByType;
        std::optional<Vector<3>> primaryAxis;
        std::optional<Vector<3>> secondaryAxis;
        Vector<3> geometricOrigin;
        double volume{};

        [[nodiscard]] double calculateVolume() const;
        [[nodiscard]] static InteractionCentreLayoutWithMetadata
        sphereDataToInteractionCentreLayoutWithMetadata(std::vector<SphereData> sphereData);

    public:
        /**
         * @brief Constructs the object.
         * @param sphereData sphere data describing all constituent monomers
         * @param primaryAxis the primary axis of the polymer
         * @param secondaryAxis the secondary axis of the polymer (should be orthogonal to the primary one)
         * @param geometricOrigin geometric origin of the molecule which can be different that the mass centre
         * @param volume volume of the polymer; if not specified, it is calculated automatically, but only for
         * non-overlapping spheres
         * @param customNamedPoints custom named points in addition to default ones (see
         * PolysphereGeometry::getNamedPoint)
         * @details This legacy-style constructor synthesizes a typed interaction-centre layout in which each sphere
         * becomes its own interaction-centre type.
         */
        explicit PolysphereGeometry(std::vector<SphereData> sphereData, OptionalAxis primaryAxis = std::nullopt,
                                    OptionalAxis secondaryAxis = std::nullopt,
                                    const Vector<3> &geometricOrigin = {0, 0, 0},
                                    std::optional<double> volume = std::nullopt,
                                    const ShapeGeometry::NamedPoints &customNamedPoints = {});

        /**
         * @brief Constructs the object from an explicit typed interaction-centre description.
         * @param interactionCentreLayoutWithMetadata interaction-centre layout together with per-type metadata
         * containing sphere radii
         * @param primaryAxis the primary axis of the polymer
         * @param secondaryAxis the secondary axis of the polymer (should be orthogonal to the primary one)
         * @param geometricOrigin geometric origin of the molecule which can be different from the mass centre
         * @param volume volume of the polymer; if not specified, it is calculated automatically, but only for
         * non-overlapping spheres reconstructed from the supplied metadata
         * @param customNamedPoints custom named points in addition to default ones (see
         * PolysphereGeometry::getNamedPoint)
         */
        explicit PolysphereGeometry(InteractionCentreLayoutWithMetadata interactionCentreLayoutWithMetadata,
                                    OptionalAxis primaryAxis = std::nullopt,
                                    OptionalAxis secondaryAxis = std::nullopt,
                                    const Vector<3> &geometricOrigin = {0, 0, 0},
                                    std::optional<double> volume = std::nullopt,
                                    const ShapeGeometry::NamedPoints &customNamedPoints = {});

        [[nodiscard]] Vector<3> getPrimaryAxis(const Shape &shape) const override {
            if (!this->primaryAxis.has_value())
                throw std::runtime_error("PolysphereGeometry::getPrimaryAxis: primary axis not defined");
            return shape.getOrientation() * this->primaryAxis.value();
        }

        [[nodiscard]] Vector<3> getSecondaryAxis(const Shape &shape) const override {
            if (!this->secondaryAxis.has_value())
                throw std::runtime_error("PolysphereGeometry::getSecondaryAxis: secondary axis not defined");
            return shape.getOrientation() * this->secondaryAxis.value();
        }

        [[nodiscard]] Vector<3> getGeometricOrigin(const Shape &shape) const override {
            return shape.getOrientation() * this->geometricOrigin;
        }

        [[nodiscard]] double getVolume() const override { return this->volume; }

        /**
         * @brief Returns bead-wise sphere representation of the polysphere.
         * @details In the typed-layout case, the vector is reconstructed from interaction-centre positions and
         * per-type metadata.
         */
        [[nodiscard]] const std::vector<SphereData> &getSphereData() const { return this->sphereData; }

        /**
         * @brief Returns the interaction-centre layout used by soft central interactions.
         */
        [[nodiscard]] const InteractionCentreLayout &getInteractionCentreLayout() const {
            return this->interactionCentreLayout;
        }

        /**
         * @brief Returns radii associated with interaction-centre types.
         * @details The order matches interaction-centre type indices used by getInteractionCentreLayout().
         */
        [[nodiscard]] const std::vector<double> &getDisplayRadiiByType() const { return this->displayRadiiByType; }

        /**
         * @brief Calculates mass centre and moves it to {0, 0, 0} (geometric origin and named points are moved
         * accordingly).
         * @details Sphere overlaps are not accounted for.
         */
        void normalizeMassCentre();

        /**
         * @brief Calculates and returns mass centre.
         * @details Sphere overlaps are not accounted for.
         */
        [[nodiscard]] Vector<3> calculateMassCentre() const;

        void setGeometricOrigin(const Vector<3> &geometricOrigin_) { this->geometricOrigin = geometricOrigin_; }

        void addCustomNamedPoints(const ShapeGeometry::NamedPoints &namedPoints) {
            this->registerNamedPoints(namedPoints);
        }

        [[nodiscard]] bool spheresOverlap() const;
    };

private:
    class HardInteraction : public Interaction {
    private:
        std::vector<Vector<3>> interactionCentres;
        std::vector<double> radii;

    public:
        explicit HardInteraction(const PolysphereGeometry &geometry);

        [[nodiscard]] bool hasHardPart() const override { return true; }
        [[nodiscard]] bool hasSoftPart() const override { return false; }
        [[nodiscard]] bool hasWallPart() const override { return true; }
        [[nodiscard]] bool isConvex() const override { return false; }
        [[nodiscard]] bool overlapBetween(const Vector<3> &pos1, const Matrix<3, 3> &orientation1, std::size_t idx1,
                                          const Vector<3> &pos2, const Matrix<3, 3> &orientation2, std::size_t idx2,
                                          const BoundaryConditions &bc) const override;
        [[nodiscard]] bool overlapWithWall(const Vector<3> &pos, const Matrix<3, 3> &orientation, std::size_t idx,
                                           const Vector<3> &wallOrigin, const Vector<3> &wallVector) const override;

        [[nodiscard]] std::vector<Vector<3>> getInteractionCentres() const override;

        [[nodiscard]] double getRangeRadius() const override;
    };

    class WolframPrinter : public ShapePrinter {
    private:
        const PolysphereTraits &traits;

    public:
        explicit WolframPrinter( const PolysphereTraits &traits) : traits{traits} { }
        [[nodiscard]] std::string print(const Shape &shape) const override;
    };

    [[nodiscard]] std::shared_ptr<ShapePrinter> createObjPrinter(std::size_t subdivisions) const;

    PolysphereGeometry geometry;
    std::shared_ptr<Interaction> interaction{};
    std::shared_ptr<WolframPrinter> wolframPrinter;

public:
    /** @brief The default number of sphere subdivisions when printing the shape (see XCPrinter::XCPrinter
     * @a subdivision parameter) */
    static constexpr std::size_t DEFAULT_MESH_SUBDIVISIONS = 3;

    /**
     * @brief Constructs a hard polysphere from the specified geometry.
     * @param geometry PolysphereGeometry describing the molecule.
     */
    explicit PolysphereTraits(PolysphereGeometry geometry);

    /**
     * @brief Constructs a polysphere with a soft central interaction bound to its interaction-centre layout.
     * @param geometry PolysphereGeometry describing the molecule
     * @param centralInteraction soft central interaction to be bound to the geometry interaction-centre layout
     * @param allowUniformPairDataBroadcast if @a true and @a centralInteraction stores pair data for a single
     * interaction-centre type only, this singular pair data is broadcast to all centre types present in @a geometry
     */
    PolysphereTraits(PolysphereGeometry geometry, std::shared_ptr<CentralInteractionBase> centralInteraction,
                     bool allowUniformPairDataBroadcast = false);

    [[nodiscard]] const Interaction &getInteraction() const override { return *this->interaction; }
    [[nodiscard]] const ShapeGeometry &getGeometry() const override { return this->geometry; }

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

    [[nodiscard]] const std::vector<SphereData> &getSphereData() const { return this->geometry.getSphereData(); }
};


#endif //RAMPACK_POLYSPHERETRAITS_H
