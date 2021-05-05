//
// Created by Piotr Kubala on 05/05/2021.
//

#ifndef RAMPACK_SMECTICORDER_H
#define RAMPACK_SMECTICORDER_H

#include <complex>

#include "core/Observable.h"

class SmecticOrder : public Observable {
private:
    std::complex<double> tau{};
    std::array<int, 3> kTau{0, 0, 0};
    std::array<int, 3> kTauRanges{};

    static std::complex<double> calculateTau(const std::array<int, 3> &kTau_, const Packing &packing);

public:
    explicit SmecticOrder(const std::array<int, 3> &kTauRanges);
    SmecticOrder() : SmecticOrder({5, 5, 5}) { }

    void calculate(const Packing &packing, double temperature, double pressure,
                   const ShapeTraits &shapeTraits) override;

    [[nodiscard]] std::vector<std::string> getIntervalHeader() const override { return {"tau"}; }
    [[nodiscard]] std::vector<std::string> getNominalHeader() const override { return {"k_tau"}; }
    [[nodiscard]] std::vector<double> getIntervalValues() const override {
        return {std::abs(this->tau)};
    }
    [[nodiscard]] std::vector<std::string> getNominalValues() const override;
    [[nodiscard]] std::string getName() const override { return "smectic order"; }
};


#endif //RAMPACK_SMECTICORDER_H
