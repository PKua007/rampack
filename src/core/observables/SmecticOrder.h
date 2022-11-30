//
// Created by Piotr Kubala on 05/05/2021.
//

#ifndef RAMPACK_SMECTICORDER_H
#define RAMPACK_SMECTICORDER_H

#include <complex>

#include "core/Observable.h"

/**
 * @brief Absolute value of (interval) smectic tau parameter.
 * @details The wavevector (nominal observable) giving maximal tau is selected automatically. The wavevectors compatible
 * with PBC are given by integer multiples of 2 pi/(box length).
 */
class SmecticOrder : public Observable {
private:
    bool dumpTauVector{};
    std::string focalPoint{};
    std::complex<double> tau{};
    std::array<int, 3> nTau{0, 0, 0};
    Vector<3> kTauVector{};
    std::array<int, 3> nTauRanges{};

    static auto calculateTau(const std::array<int, 3> &nTau_, const Packing &packing,
                             const std::vector<Vector<3>> &focalPoints);

public:
    /**
     * @brief Constructor with @a nRanges - a specified range of integer multiples of 2 pi/(box length) to construct
     * wavevectors.
     * @details If @a dumpTauVector_ is @a true, components of tau wavevector will also be dumped as interval values.
     * Moreover, @a focalPoint specifies which named point (see ShapeGeometry::getNamedPointForShape) should be used to
     * calculate smectic order.
     */
    explicit SmecticOrder(const std::array<int, 3> &nTauRanges = {5, 5, 5}, bool dumpTauVector_ = false,
                          std::string focalPoint = "o");

    void calculate(const Packing &packing, double temperature, double pressure,
                   const ShapeTraits &shapeTraits) override;

    [[nodiscard]] std::vector<std::string> getIntervalHeader() const override;
    [[nodiscard]] std::vector<std::string> getNominalHeader() const override { return {"n_tau"}; }
    [[nodiscard]] std::vector<double> getIntervalValues() const override;
    [[nodiscard]] std::vector<std::string> getNominalValues() const override;
    [[nodiscard]] std::string getName() const override { return "smectic order"; }
};


#endif //RAMPACK_SMECTICORDER_H
