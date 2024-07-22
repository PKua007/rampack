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


/**
 * @brief A polymer consisting of identical or different hard of soft-interacting spheres.
 */
template <typename ConcreteTraits>
class PolysphereTraits : public ShapeTraits {
private:
    class HardInteraction : public Interaction {
    private:
        const PolysphereTraits &traits;

    public:
        explicit HardInteraction(const PolysphereTraits &traits) : traits{traits} { }

        [[nodiscard]] bool hasHardPart() const override {
            return true;
        }
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

protected:
    CentralInteraction &getCentralInteraction() { return *this->centralInteraction; }

public:
    /**
     * @brief The default number of sphere subdivisions when printing the shape in the OBJ format (see
     * XCPrinter::buildPolyhedron @a subdivisions parameter)
     */
    static constexpr std::size_t DEFAULT_MESH_SUBDIVISIONS = 3;

    PolysphereTraits();
    explicit PolysphereTraits(const std::shared_ptr<CentralInteraction> &centralInteraction);

    PolysphereTraits(const PolysphereTraits &) = delete;
    PolysphereTraits &operator=(const PolysphereTraits &) = delete;
    ~PolysphereTraits() override;

    [[nodiscard]] const Interaction &getInteraction() const override { return *this->interaction; }

    [[nodiscard]] std::vector<Vector<3>> getInteractionCentres(const std::byte *data) const;

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


#include "PolysphereTraits.tpp"

#endif //RAMPACK_POLYSPHERETRAITS_H
