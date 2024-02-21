//
// Created by Piotr Kubala on 20/12/2020.
//

#ifndef RAMPACK_SPHERETRAITS_H
#define RAMPACK_SPHERETRAITS_H

#include <optional>

#include "core/ShapeTraits.h"
#include "core/interactions/CentralInteraction.h"


/**
 * @brief Spherical molecules with hard or soft interactions.
 */
class SphereTraits : public ShapeTraits, public ShapeGeometry {
public:
    struct HardData {
        double radius{};

        friend bool operator==(HardData lhs, HardData rhs) { return lhs.radius == rhs.radius; }
    };

private:
    class HardDataManager : public ShapeDataManager {
    public:
        explicit HardDataManager(std::optional<double> defaultRadius);

        [[nodiscard]] size_t getShapeDataSize() const override { return sizeof(HardData); }
        void validateShapeData(const ShapeData &data) const override;
        [[nodiscard]] TextualShapeData serialize(const ShapeData &data) const override;
        [[nodiscard]] ShapeData deserialize(const TextualShapeData &data) const override;

        [[nodiscard]] ShapeData::Comparator getComparator() const override {
            return ShapeData::Comparator::forType<HardData>();
        }
    };

    class HardInteraction : public Interaction {
    public:
        [[nodiscard]] bool hasHardPart() const override { return true; }
        [[nodiscard]] bool hasSoftPart() const override { return false; }
        [[nodiscard]] bool hasWallPart() const override { return true; }
        [[nodiscard]] bool isConvex() const override { return true; }
        [[nodiscard]] bool overlapBetween(const Vector<3> &pos1, const Matrix<3, 3> &orientation1,
                                          const std::byte *data1, std::size_t idx1, const Vector<3> &pos2,
                                          const Matrix<3, 3> &orientation2, const std::byte *data2, std::size_t idx2,
                                          const BoundaryConditions &bc) const override;
        [[nodiscard]] bool overlapWithWall(const Vector<3> &pos, const Matrix<3, 3> &orientation, const std::byte *data,
                                           std::size_t idx, const Vector<3> &wallOrigin,
                                           const Vector<3> &wallVector) const override;
        [[nodiscard]] double getRangeRadius([[maybe_unused]] const std::byte *data) const override;
    };

    class WolframPrinter : public ShapePrinter {
    private:
        std::optional<double> fixedRadius{};

    public:
        explicit WolframPrinter() = default;
        explicit WolframPrinter(double fixedRadius);
        [[nodiscard]] std::string print(const Shape &shape) const override;
    };

    static std::shared_ptr<ShapePrinter> createObjPrinter(double radius, std::size_t subdivisions);

    std::optional<double> fixedRadius{};
    std::shared_ptr<Interaction> interaction;
    std::shared_ptr<ShapeDataManager> dataManager;
    std::shared_ptr<WolframPrinter> wolframPrinter;

public:
    /** @brief The default number of sphere subdivisions when printing the shape (see XCPrinter::XCPrinter
     * @a subdivision parameter) */
    static constexpr std::size_t DEFAULT_MESH_SUBDIVISIONS = 3;

    /**
     * @brief Creates a hard sphere.
     */
    explicit SphereTraits(std::optional<double> defaultRadius = std::nullopt);

    /**
     * @brief Creates a sphere interacting via @a centralInteraction soft potential.
     */
    SphereTraits(double fixedRadius, std::shared_ptr<CentralInteraction> centralInteraction);

    [[nodiscard]] const Interaction &getInteraction() const override { return *this->interaction; };

    /**
     * @brief Returns ShapePrinter for a given @a format.
     * @details The following formats are supported:
     * <ol>
     *     <li> `wolfram` - Wolfram Mathematica shape
     *     <li> `obj` - Wavefront OBJ triangle mesh (it accepts @a mesh_divisions parameter, default: 4)
     * </ol>
     */
    [[nodiscard]] std::shared_ptr<const ShapePrinter>
    getPrinter(const std::string &format, const std::map<std::string, std::string> &params) const override;

    [[nodiscard]] const ShapeGeometry &getGeometry() const override { return *this; }
    [[nodiscard]] double getVolume(const Shape &shape) const override;

    [[nodiscard]] const ShapeDataManager &getDataManager() const override { return *this->dataManager; }
};


#endif //RAMPACK_SPHERETRAITS_H
