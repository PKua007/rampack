//
// Created by ciesla on 12/30/22.
//

#ifndef RAMPACK_EVBOX_H
#define RAMPACK_EVBOX_H


#include <random>
#include "core/FreeBoundaryConditions.h"
#include "core/Shape.h"
#include "core/Interaction.h"

class EVBox {
private:
    static std::uniform_real_distribution<double> u01Distribution;
    static std::mt19937 mt;
    static FreeBoundaryConditions fbc;

    Vector<3> position{}; // (left, down, front) position
    double length{};
    std::size_t samples{};
    std::size_t intersections{};

public:
    EVBox(const Vector<3> &v, double l);

    void divide(std::vector<EVBox *> &newBoxes) const;
    void sampleCorners(const Shape& originShape, Shape testShape, const Interaction &interaction);
    void sampleMC(Matrix<3, 3, double> *orientation, const Shape& originShape, const Shape& testShape, const Interaction &interaction, size_t samples);
    const Vector<3>& getPosition() const;

    double volume() const;
    std::size_t getIntersections() const;
    std::size_t getSamples() const;
    double getCoverage() const;
};


#endif //RAMPACK_EVBOX_H
