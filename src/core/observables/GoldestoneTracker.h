//
// Created by pkua on 20.10.22.
//

#ifndef RAMPACK_GOLDESTONETRACKER_H
#define RAMPACK_GOLDESTONETRACKER_H

#include "core/Observable.h"


class GoldestoneTracker : public Observable {
protected:
    Vector<3> originPos;
    Matrix<3, 3> originRot = Matrix<3, 3>::identity();

public:
    [[nodiscard]] virtual std::string getModeName() const = 0;
    virtual void calculateOrigin(const Packing &packing, const ShapeTraits &shapeTraits) = 0;

    void calculate(const Packing &packing, double temperature, double pressure, const ShapeTraits &shapeTraits) final;
    [[nodiscard]] std::vector<std::string> getIntervalHeader() const final;
    [[nodiscard]] std::vector<std::string> getNominalHeader() const final { return {}; }
    [[nodiscard]] std::vector<double> getIntervalValues() const final;
    [[nodiscard]] std::vector<std::string> getNominalValues() const final { return {}; }
    [[nodiscard]] std::string getName() const final { return this->getModeName() + "_tracker"; }

    [[nodiscard]] const Vector<3> &getOriginPos() const { return this->originPos; }
    [[nodiscard]] const Matrix<3, 3> &getOriginRot() const { return this->originRot; }
};




#endif //RAMPACK_GOLDESTONETRACKER_H
