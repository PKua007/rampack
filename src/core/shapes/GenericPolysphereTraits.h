//
// Created by Piotr Kubala on 22/07/2024.
//

#ifndef RAMPACK_GENERICPOLYSPHERETRAITS_H
#define RAMPACK_GENERICPOLYSPHERETRAITS_H

#include "PolysphereTraits.h"
#include "GenericShapeRegistry.h"
#include "OptionalAxis.h"


/**
 * @brief Class representing a single species of a polysphere for PolysphereTraits, conforming to GenericShapeRegistry
 * @a ConcreteSpecies template parameter.
 */
class PolysphereShape /* : public GenericShapeRegistry::ConcreteSpecies */ {
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


class GenericPolysphereTraits
        : public PolysphereTraits<GenericPolysphereTraits>, public GenericShapeRegistry<PolysphereShape>
{
public:
    /**
     * @brief Creates the class with hard-core interactions and no initially registered species.
     */
    GenericPolysphereTraits();

    /**
     * @brief Creates the class with hard-core interactions and one initial species @a defaultSpecies named `A`, which
     * is set as a default species (setDefaultSpecies()).
     */
    explicit GenericPolysphereTraits(const PolysphereShape &defaultSpecies);

    /**
     * @brief Creates the class with soft interactions @a centralInteraction and no initially registered species.
     */
    explicit GenericPolysphereTraits(const std::shared_ptr<CentralInteraction> &centralInteraction);

    /**
     * @brief Creates the class with soft interactions @a centralInteraction and one species @a shape named `A`, which
     * is set as a default species (setDefaultSpecies()).
     */
    GenericPolysphereTraits(const PolysphereShape &polysphereShape,
                            const std::shared_ptr<CentralInteraction> &centralInteraction);

    [[nodiscard]] const std::vector<SphereData> &getSphereData(const std::byte *data) const {
        return this->speciesFor(data).getSphereData();
    }

    [[nodiscard]] const ShapeGeometry &getGeometry() const override { return *this; }
    [[nodiscard]] const ShapeDataManager &getDataManager() const override { return *this; }
};


#endif //RAMPACK_GENERICPOLYSPHERETRAITS_H
