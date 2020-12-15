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

class Packing {
private:
    std::vector<std::unique_ptr<Shape>> shapes;
    double linearSize{};
    std::unique_ptr<BoundaryConditions> bc;

    [[nodiscard]] bool areAnyParticlesOverlapping() const;
    [[nodiscard]] bool isAnyParticleCollidingWith(std::size_t i) const;

public:
    using iterator = decltype(shapes)::iterator;
    using const_iterator = decltype(shapes)::const_iterator;

    Packing(double linearSize, std::vector<std::unique_ptr<Shape>> shapes, std::unique_ptr<BoundaryConditions> bc);

    bool tryTranslation(std::size_t particleIdx, std::array<double, 3> translation);
    bool tryScaling(double scaleFactor);

    [[nodiscard]] double getLinearSize() const { return linearSize; }

    [[nodiscard]] std::size_t size() const { return this->shapes.size(); }
    [[nodiscard]] bool empty() const { return this->shapes.empty(); }
    [[nodiscard]] const_iterator begin() const { return this->shapes.begin(); }
    [[nodiscard]] const_iterator end() const { return this->shapes.end(); }
    [[nodiscard]] const Shape &operator[](std::size_t i);

    void toWolfram(std::ostream &out) const;
    [[nodiscard]] double getPackingFraction() const;
    [[nodiscard]] double getNumberDensity() const;
};


#endif //RAMPACK_PACKING_H
