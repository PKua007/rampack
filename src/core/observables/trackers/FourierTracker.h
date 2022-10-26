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

    static constexpr double AMPLITUDE_EPSILON = 1e-8;

    std::array<std::size_t, 3> wavenumbers{};
    Function function;
    std::string functionName;
    FourierFunctions fourierFunctions{};
    std::vector<std::size_t> nonzeroIdxs{};

    [[nodiscard]] FourierValues calculateFourierValues(const Packing &packing, const ShapeTraits &shapeTraits) const;
    [[nodiscard]] Vector<3> calculateRelativeOriginPos(const FourierValues &fourierValues) const;
    [[nodiscard]] Vector<3> calculateRelativeOriginPos1D(const FourierValues &fourierValues) const;
    [[nodiscard]] Vector<3> calculateRelativeOriginPos2D(const FourierValues &fourierValues) const;

public:
    FourierTracker(const std::array<std::size_t, 3> &wavenumbers, Function function, std::string functionName);

    [[nodiscard]] std::string getModeName() const override;
    void calculateOrigin(const Packing &packing, const ShapeTraits &shapeTraits) override;
};


#endif //RAMPACK_FOURIERTRACKER_H
