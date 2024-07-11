//
// Created by Piotr Kubala on 20/12/2020.
//

#ifndef RAMPACK_SPHEROCYLINDERTRAITS_H
#define RAMPACK_SPHEROCYLINDERTRAITS_H

#include <optional>

#include "core/ShapeTraits.h"


/**
 * @brief Hard spherocylinder spanned on Z axis.
 * @details The primary axis is naturally the Z axis. Mass center coincides with geometric origin. The class, apart from
 * standard named points (see ShapeGeometry::getNamedPoint()) defines points "beg" and "end" representing spherical cap
 * centers lying, respectively, on the negative and the positive Z axis.
 */
class SpherocylinderTraits : public ShapeTraits, public Interaction, public ShapeGeometry, public ShapeDataManager {
private:
    class WolframPrinter : public ShapePrinter {
    public:
        [[nodiscard]] std::string print(const Shape &shape) const override;
    };

    std::optional<double> defaultLength{};
    std::optional<double> defaultRadius{};
    std::shared_ptr<WolframPrinter> wolframPrinter;

    static std::unique_ptr<ShapePrinter> createObjPrinter(std::size_t subdivisions);
    static Vector<3> getCapCentre(short beginOrEnd, const Matrix<3, 3> &rot, double length);
    static Vector<3> getCapCentre(short beginOrEnd, const Shape &shape);
    static Vector<3> getCapCentre(short beginOrEnd, const ShapeData &data);

    friend WolframPrinter;

public:
    /**
     * @brief ShapeData -compatible structure with spherocylinder data.
     */
    struct Data {
        /** @brief Distance between the spherical caps' centers. It must be non-negative. */
        double length{};

        /** @brief Radius of the spherical caps (half-width of the spherocylinder). It must be positive. */
        double radius{};

        [[nodiscard]] friend bool operator==(const Data &lhs, const Data &rhs) {
            return std::tie(lhs.length, lhs.radius) == std::tie(rhs.length, rhs.radius);
        }
    };

    /**
     * @brief The default number of sphere subdivisions when printing the shape in the OBJ format (see
     * XCPrinter::buildPolyhedron @a subdivisions parameter)
     */
    static constexpr std::size_t DEFAULT_MESH_SUBDIVISIONS = 3;

    /**
     * @brief Creates a hard spherocylinder, supporting polydispersity.
     * @param defaultLength if not `std::nullopt`, it defines the default value of Data::length used by
     * ShapeDataManager::defaultSerialize and ShapeDataManager::defaultDeserialize
     * @param defaultRadius if not `std::nullopt`, it defines the default value of Data::radius used by
     * ShapeDataManager::defaultSerialize and ShapeDataManager::defaultDeserialize
     * @throws PreconditionException if @a defaultLength is set and negative, or if @a defaultRadius is set and
     * non-positive
     */
    explicit SpherocylinderTraits(std::optional<double> defaultLength = std::nullopt,
                                  std::optional<double> defaultRadius = std::nullopt);

    [[nodiscard]] const Interaction &getInteraction() const override { return *this; }
    [[nodiscard]] const ShapeGeometry &getGeometry() const override { return *this; }
    [[nodiscard]] const ShapeDataManager &getDataManager() const override { return *this; }

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

    [[nodiscard]] Vector<3> getPrimaryAxis(const Shape &shape) const override;
    [[nodiscard]] double getVolume(const Shape &shape) const override;

    [[nodiscard]] bool hasHardPart() const override { return true; }
    [[nodiscard]] bool hasWallPart() const override { return true; }
    [[nodiscard]] bool hasSoftPart() const override { return false; }
    [[nodiscard]] bool isConvex() const override { return true; }
    [[nodiscard]] bool overlapBetween(const Vector<3> &pos1, const Matrix<3, 3> &orientation1, const std::byte *data1,
                                      std::size_t idx1, const Vector<3> &pos2, const Matrix<3, 3> &orientation2,
                                      const std::byte *data2, std::size_t idx2,
                                      const BoundaryConditions &bc) const override;

    [[nodiscard]] bool overlapWithWall(const Vector<3> &pos, const Matrix<3, 3> &orientation, const std::byte *data,
                                       std::size_t idx, const Vector<3> &wallOrigin,
                                       const Vector<3> &wallVector) const override;

    [[nodiscard]] double getRangeRadius(const std::byte *data) const override {
        const auto &scData = ShapeData::as<Data>(data);
        return 2*scData.radius + scData.length;
    }

    [[nodiscard]] std::size_t getShapeDataSize() const override { return sizeof(Data); }

    /**
     * @brief Validates if the @a data, interpreted as Data, are correct, i.e. Data::length is non-negative and
     * Data::radius is positive. If validation fails, ShapeDataFormatException is thrown.
     */
    void validateShapeData(const ShapeData &data) const override;

    [[nodiscard]] ShapeData::Comparator getComparator() const override {
        return ShapeData::Comparator::forType<Data>();
    }

    /**
     * @brief Serializes the spherocylinder into a map with shape parameters named `length` and `radius`, corresponding
     * to the ones of Data.
     */
    [[nodiscard]] TextualShapeData serialize(const ShapeData &data) const override;

    /**
     * @brief Deserializes the spherocylinder from a map with shape parameters named `length` and `radius`,
     * corresponding to the ones of Data.
     * @throws ShapeDataSerializationException if the keys are incorrect or the values are not numbers
     * @throws ShapeDataFormatException if the resulting values would not be validated as correct by validateShapeData()
     */
    [[nodiscard]] ShapeData deserialize(const TextualShapeData &data) const override;
};


#endif //RAMPACK_SPHEROCYLINDERTRAITS_H
