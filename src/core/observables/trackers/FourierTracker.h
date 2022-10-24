//
// Created by pkua on 20.10.22.
//

#ifndef RAMPACK_FOURIERTRACKER_H
#define RAMPACK_FOURIERTRACKER_H

#include <array>
#include <functional>

#include "core/observables/GoldestoneTracker.h"


class FourierTracker : GoldestoneTracker {
public:
    using Function = std::function<double(const Shape &, const ShapeTraits &)>;

private:
    using FourierFunction = std::function<double(double)>;
    using FourierFunctions = std::array<std::vector<FourierFunction>, 3>;
    using FourierValues = std::array<std::array<std::array<double, 2>, 2>, 2>;

    std::array<std::size_t, 3> wavenumbers{};
    Function function;
    std::string functionName;
    FourierFunctions fourierFunctions{};

    [[nodiscard]] FourierValues calculateFourierValues(const Packing &packing, const ShapeTraits &shapeTraits) const;

public:
    FourierTracker(const std::array<std::size_t, 3> &wavenumber, Function function, std::string functionName);

    [[nodiscard]] std::string getModeName() const override;
    void calculateOrigin(const Packing &packing, const ShapeTraits &shapeTraits) override;
};


#endif //RAMPACK_FOURIERTRACKER_H
