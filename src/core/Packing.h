//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_PACKING_H
#define RAMPACK_PACKING_H

#include <vector>
#include <memory>
#include <array>

#include "Shape.h"
#include "BoundaryConditions.h"
#include "Interaction.h"
#include "ShapePrinter.h"

class Packing {
private:
    std::vector<std::unique_ptr<Shape>> shapes;
    double linearSize{};
    std::unique_ptr<BoundaryConditions> bc;

    std::size_t lastAlteredParticleIdx{};
    Vector<3> lastTranslation;
    Matrix<3, 3> lastRotation;
    double lastScalingFactor{};

    [[nodiscard]] bool areAnyParticlesOverlapping(const Interaction &interaction) const;
    [[nodiscard]] bool isAnyParticleCollidingWith(std::size_t i, const Interaction &interaction) const;

public:
    using iterator = decltype(shapes)::iterator;
    using const_iterator = decltype(shapes)::const_iterator;

    Packing(double linearSize, std::vector<std::unique_ptr<Shape>> shapes, std::unique_ptr<BoundaryConditions> bc);

    double tryTranslation(std::size_t particleIdx, Vector<3> translation, const Interaction &interaction);
    double tryRotation(std::size_t particleIdx, const Matrix<3, 3> &rotation, const Interaction &interaction);
    double tryScaling(double scaleFactor, const Interaction &interaction);
    void revertTranslation();
    void revertRotation();
    void revertScaling();
    [[nodiscard]] double getParticleEnergy(std::size_t particleIdx, const Interaction &interaction) const;
    [[nodiscard]] double getParticleEnergyFluctuations(const Interaction &interaction) const;
    [[nodiscard]] double getTotalEnergy(const Interaction &interaction) const;

    [[nodiscard]] double getLinearSize() const { return linearSize; }

    [[nodiscard]] std::size_t size() const { return this->shapes.size(); }
    [[nodiscard]] bool empty() const { return this->shapes.empty(); }
    [[nodiscard]] const_iterator begin() const { return this->shapes.begin(); }
    [[nodiscard]] const_iterator end() const { return this->shapes.end(); }
    [[nodiscard]] const Shape &operator[](std::size_t i) const;
    [[nodiscard]] const Shape &front() const;
    [[nodiscard]] const Shape &back() const;

    void toWolfram(std::ostream &out, const ShapePrinter &printer) const;
    [[nodiscard]] double getPackingFraction(double shapeVolume) const;
    [[nodiscard]] double getNumberDensity() const;
};


#endif //RAMPACK_PACKING_H
