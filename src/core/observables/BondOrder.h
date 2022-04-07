//
// Created by pkua on 05.04.2022.
//

#ifndef RAMPACK_BONDORDER_H
#define RAMPACK_BONDORDER_H

#include "core/Observable.h"


class BondOrder : public Observable {
private:
    std::vector<std::size_t> ranks;
    std::vector<double> psis;
    std::vector<std::string> header;
    std::array<int, 3> layerWavenumber;

    static void insertDistance(std::vector<std::pair<std::size_t, double>> &vector, std::size_t particleIdx,
                               double distance2);

public:
    BondOrder(std::size_t rank, const std::array<int, 3> &layerWavenumber)
            : BondOrder(std::vector<std::size_t>{rank}, layerWavenumber)
    { }

    BondOrder(std::vector<std::size_t> ranks, const std::array<int, 3> &layerWavenumber);

    void calculate(const Packing &packing, double temperature, double pressure,
                   const ShapeTraits &shapeTraits) override;

    [[nodiscard]] std::vector<std::string> getIntervalHeader() const override { return this->header; }
    [[nodiscard]] std::vector<std::string> getNominalHeader() const override { return {}; }
    [[nodiscard]] std::vector<double> getIntervalValues() const override { return this->psis; }
    [[nodiscard]] std::vector<std::string> getNominalValues() const override { return {}; }
    [[nodiscard]] std::string getName() const override { return "bond order"; }
};


#endif //RAMPACK_BONDORDER_H
