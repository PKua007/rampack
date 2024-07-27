//
// Created by Piotr Kubala on 27/07/2024.
//

#ifndef RAMPACK_SPECIESSHAPEGEOMETRY_H
#define RAMPACK_SPECIESSHAPEGEOMETRY_H

#include "core/ShapeGeometry.h"


template <typename ConcreteSpeciesRegistry>
class SpeciesShapeGeometry : public ShapeGeometry {
private:
    [[nodiscard]] const ConcreteSpeciesRegistry &concreteSpeciesRegistry() const {
        return static_cast<const ConcreteSpeciesRegistry &>(*this);
    }

public:
    SpeciesShapeGeometry() {
        NamedPoint::TransientEvaluator evaluator = [this](const std::string &pointName, const ShapeData &data) {
            const auto &species = this->concreteSpeciesRegistry().speciesFor(data);
            const std::map<std::string, Vector<3>> &namedPoints = species.getNamedPoints();

            auto it = namedPoints.find(pointName);
            if (it == namedPoints.end())
                this->concreteSpeciesRegistry().throwUnavailableNamedPoint(pointName, data);

            return it->second;
        };

        NamedPoint::TransientLister lister = [this](const ShapeData &data) {
            const auto &species = this->concreteSpeciesRegistry().speciesFor(data);
            const std::map<std::string, Vector<3>> &namedPoints = species.getNamedPoints();

            std::set<std::string> pointNames;
            for (const auto &[pointName, pointCoords] : namedPoints)
                pointNames.insert(pointName);
            return pointNames;
        };

        this->registerTransientNamedPoint(std::move(evaluator), std::move(lister));
    }

    /**
     * @brief Return shape's primary axis based on `ConcreteSpecies::getPrimaryAxis`.
     */
    [[nodiscard]] Vector<3> getPrimaryAxis(const Shape &shape) const final {
        const auto &species = this->concreteSpeciesRegistry().speciesFor(shape);
        return shape.getOrientation() * species.getPrimaryAxis();
    }

    /**
     * @brief Return shape's secondary axis based on `ConcreteSpecies::getSecondaryAxis`.
     */
    [[nodiscard]] Vector<3> getSecondaryAxis(const Shape &shape) const final {
        const auto &species = this->concreteSpeciesRegistry().speciesFor(shape);
        return shape.getOrientation() * species.getSecondaryAxis();
    }

    /**
     * @brief Return shape's geometric origin based on `ConcreteSpecies::getGeometricOrigin`.
     */
    [[nodiscard]] Vector<3> getGeometricOrigin(const Shape &shape) const final {
        const auto &species = this->concreteSpeciesRegistry().speciesFor(shape);
        return shape.getOrientation() * species.getGeometricOrigin();
    }

    /**
     * @brief Return shape's volume based on `ConcreteSpecies::getVolume`.
     */
    [[nodiscard]] double getVolume(const Shape &shape) const final {
        return this->concreteSpeciesRegistry().speciesFor(shape).getVolume();
    }
};


#endif //RAMPACK_SPECIESSHAPEGEOMETRY_H
