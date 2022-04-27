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
    std::complex<double> tau{};
    std::array<int, 3> kTau{0, 0, 0};
    Vector<3> kTauVector{};
    std::array<int, 3> kTauRanges{};

    static auto calculateTau(const std::array<int, 3> &kTau_, const Packing &packing);

public:
    /**
     * @brief Constructor with @a kTauRanges - a specified range of integer multiples of 2 pi/(box length) to construct
     * wavevectors.
     * @detail If @a dumpTauVector_ is @a true, components of tau wavevector will also be dumped as interval values.
     */
    explicit SmecticOrder(const std::array<int, 3> &kTauRanges, bool dumpTauVector_ = false);

    /**
     * @brief Constructor with a default range of integer multiples of 2 pi/(box length) to construct wavevectors.
     * @detail If @a dumpTauVector_ is @a true, components of tau wavevector will also be dumped as interval values.
     */
    explicit SmecticOrder(bool dumpTauVector_ = false) : SmecticOrder({5, 5, 5}, dumpTauVector_) { }

    void calculate(const Packing &packing, double temperature, double pressure,
                   const ShapeTraits &shapeTraits) override;

    [[nodiscard]] std::vector<std::string> getIntervalHeader() const override;
    [[nodiscard]] std::vector<std::string> getNominalHeader() const override { return {"k_tau"}; }
    [[nodiscard]] std::vector<double> getIntervalValues() const override;
    [[nodiscard]] std::vector<std::string> getNominalValues() const override;
    [[nodiscard]] std::string getName() const override { return "smectic order"; }
};


#endif //RAMPACK_SMECTICORDER_H
