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
#include "core/interactions/CentralInteraction.h"
#include "OptionalAxis.h"


class PolysphereShape {
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

private:
    std::vector<SphereData> sphereData{};
    std::optional<Vector<3>> primaryAxis;
    std::optional<Vector<3>> secondaryAxis;
    Vector<3> geometricOrigin;
    double volume{};
    std::map<std::string, Vector<3>> customNamedPoints;

    [[nodiscard]] double calculateVolume() const;

public:
    explicit PolysphereShape(std::vector<SphereData> sphereData, OptionalAxis primaryAxis = std::nullopt,
                             OptionalAxis secondaryAxis = std::nullopt,
                             const Vector<3> &geometricOrigin = {0, 0, 0},
                             std::optional<double> volume = std::nullopt,
                             const std::map<std::string, Vector<3>> &customNamedPoints = {});

    [[nodiscard]] Vector<3> getPrimaryAxis() const {
        if (!this->primaryAxis.has_value())
            throw std::runtime_error("PolysphereShape::getPrimaryAxis: primary axis not defined");
        return this->primaryAxis.value();
    }

    [[nodiscard]] Vector<3> getSecondaryAxis() const {
        if (!this->secondaryAxis.has_value())
            throw std::runtime_error("PolysphereShape::getSecondaryAxis: secondary axis not defined");
        return this->secondaryAxis.value();
    }

    [[nodiscard]] Vector<3> getGeometricOrigin() const {
        return this->geometricOrigin;
    }

    [[nodiscard]] double getVolume([[maybe_unused]] const Shape &shape) const { return this->volume; }
    [[nodiscard]] const std::vector<SphereData> &getSphereData() const { return this->sphereData; }
    [[nodiscard]] std::vector<Vector<3>> getInteractionCentres() const;

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
    void addCustomNamedPoints(std::map<std::string, Vector<3>> namedPoints);
    [[nodiscard]] const std::map<std::string, Vector<3>> &getCustomNamedPoints() const {
        return this->customNamedPoints;
    }
    [[nodiscard]] bool spheresOverlap() const;

    friend bool operator==(const PolysphereShape &lhs, const PolysphereShape &rhs);
};


/**
 * @brief A polymer consisting of identical or different hard of soft-interacting spheres.
 */
class PolysphereTraits : public ShapeTraits, public ShapeGeometry, public ShapeDataManager {
public:
    struct Data {
        std::size_t shapeIdx{};

        friend bool operator==(Data lhs, Data rhs) { return lhs.shapeIdx == rhs.shapeIdx; }
    };

    using SphereData = PolysphereShape::SphereData;

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

        [[nodiscard]] double getRangeRadius([[maybe_unused]] const std::byte *data) const override;
    };

    class WolframPrinter : public ShapePrinter {
    private:
        const PolysphereTraits &traits;

    public:
        explicit WolframPrinter(const PolysphereTraits &traits) : traits{traits} { }

        [[nodiscard]] std::string print(const Shape &shape) const override;
    };

    // [[nodiscard]] std::shared_ptr<ShapePrinter> createObjPrinter(std::size_t subdivisions) const;

    std::map<std::string, std::size_t> shapeNameIdxMap;
    std::vector<PolysphereShape> shapes;
    std::shared_ptr<Interaction> interaction;
    std::shared_ptr<CentralInteraction> centralInteraction;
    std::shared_ptr<WolframPrinter> wolframPrinter;

    friend HardInteraction;
    friend WolframPrinter;

    [[nodiscard]] const PolysphereShape &polysphereShapeFor(const Shape &shape) const {
        return this->polysphereShapeFor(shape.getData());
    }

    [[nodiscard]] const PolysphereShape &polysphereShapeFor(const ShapeData &data) const;

    [[nodiscard]] const PolysphereShape &polysphereShapeFor(const std::byte *data) const {
        std::size_t shapeIdx = ShapeData::as<Data>(data).shapeIdx;
        return this->shapes[shapeIdx];
    }

    void registerCustomNamedPoint(const std::string &pointName);
    void registerSphereNamedPoint(std::size_t sphereIdx);
    void throwUnavailableNamedPoint(std::size_t shapeIdx, const std::string &pointName) const;

public:
    /** @brief The default number of sphere subdivisions when printing the shape (see XCPrinter::XCPrinter
     * @a subdivision parameter) */
    static constexpr std::size_t DEFAULT_MESH_SUBDIVISIONS = 3;


    PolysphereTraits();

    /**
     * @brief Construct the polymer from the specified @a sphereData.
     * @param geometry PolysphereGeometry describing the molecule.
     */
    explicit PolysphereTraits(const PolysphereShape &polysphereShape);

    explicit PolysphereTraits(const std::shared_ptr<CentralInteraction> &centralInteraction);

    /**
     * @brief Similar as PolysphereTraits::PolysphereTraits(const std::vector<SphereData> &, const Vector<3> &, bool),
     * but for soft central interaction given by @a centralInteraction.
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

    [[nodiscard]] Vector<3> getPrimaryAxis(const Shape &shape) const override;
    [[nodiscard]] Vector<3> getSecondaryAxis(const Shape &shape) const override;
    [[nodiscard]] Vector<3> getGeometricOrigin(const Shape &shape) const override;
    [[nodiscard]] double getVolume(const Shape &shape) const override;

    [[nodiscard]] std::size_t getShapeDataSize() const override { return sizeof(Data); }
    void validateShapeData(const ShapeData &data) const override;
    [[nodiscard]] ShapeData::Comparator getComparator() const override {
        return ShapeData::Comparator::forType<Data>();
    }
    [[nodiscard]] TextualShapeData serialize(const ShapeData &data) const override;
    [[nodiscard]] ShapeData deserialize(const TextualShapeData &data) const override;

    ShapeData addPolysphereShape(const std::string &shapeName, const PolysphereShape &shape);
    [[nodiscard]] const PolysphereShape &getDefaultPolysphereShape() const;
    void setDefaultPolysphereShape(const std::string &shapeName);

    [[nodiscard]] bool hasPolysphereShape(const std::string &shapeName) const;
    [[nodiscard]] const PolysphereShape &getPolysphereShape(const std::string &shapeName) const;
    [[nodiscard]] const PolysphereShape &getPolysphereShape(std::size_t shapeIdx) const;
    [[nodiscard]] ShapeData shapeDataFor(const std::string &shapeName) const;

    [[nodiscard]] std::size_t getPolysphereShapeIdx(const std::string &shapeName) const;
    [[nodiscard]] const std::string &getPolysphereShapeName(std::size_t shapeIdx) const;
};


#endif //RAMPACK_POLYSPHERETRAITS_H
