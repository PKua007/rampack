//
// Created by pkua on 20.10.22.
//

#ifndef RAMPACK_FOURIERTRACKER_H
#define RAMPACK_FOURIERTRACKER_H

#include <array>
#include <functional>
#include <optional>
#include <set>

#include "core/observables/GoldestoneTracker.h"


class FourierTracker : public GoldestoneTracker {
public:
    using Function = std::function<double(const Shape &, const ShapeTraits &)>;

private:
    using FourierFunction = std::function<double(double)>;
    using FourierFunctions = std::array<std::vector<FourierFunction>, 3>;
    using FourierCoefficients = std::array<std::array<std::array<double, 2>, 2>, 2>;

    enum : std::size_t {
        COS = 0,
        SIN = 1
    };

    static constexpr double AMPLITUDE_EPSILON = 1e-8;

    std::array<std::size_t, 3> wavenumbers{};
    Function function;
    std::string functionName;
    FourierFunctions fourierFunctions{};
    std::vector<std::size_t> nonzeroWavenumberIdxs{};
    Vector<3> previousRelValue;

    static double guardedAtan2(double y, double x);
    static double getCoefficient(const FourierCoefficients &coefficients, const std::array<std::size_t, 3> &idxs);

    void fillFourierFunctions();
    [[nodiscard]] FourierCoefficients calculateFourierCoefficients(const Packing &packing,
                                                                   const ShapeTraits &shapeTraits) const;
    [[nodiscard]] Vector<3> calculateRelativeOriginPos(const FourierCoefficients &coefficients) const;
    [[nodiscard]] Vector<3> calculateRelativeOriginPos1D(const FourierCoefficients &coefficients) const;
    [[nodiscard]] Vector<3> calculateRelativeOriginPos2D(const FourierCoefficients &coefficients) const;
    [[nodiscard]] Vector<3> calculateRelativeOriginPos3D(const FourierCoefficients &coefficients) const;
    [[nodiscard]] Vector<3> normalizeOriginPos(const Vector<3> &originPosRel);
    [[nodiscard]] std::array<double, 7> calculateTanZCoefficients(double U, double V, double W, double X, double Y,
                                                                  double Z) const;
    [[nodiscard]] std::pair<double, double> findBestSinCosZ(const std::set<double> &roots, double U, double V, double W,
                                                            double X, double Y, double Z) const;

public:
    FourierTracker(const std::array<std::size_t, 3> &wavenumbers, Function function, std::string functionName);

    [[nodiscard]] std::string getModeName() const override;
    void calculateOrigin(const Packing &packing, const ShapeTraits &shapeTraits) override;

};


#endif //RAMPACK_FOURIERTRACKER_H
