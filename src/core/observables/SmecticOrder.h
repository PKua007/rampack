//
// Created by Piotr Kubala on 05/05/2021.
//

#ifndef RAMPACK_SMECTICORDER_H
#define RAMPACK_SMECTICORDER_H

#include <complex>

#include "core/Observable.h"
#include "core/observables/ShapeFunction.h"
#include "core/observables/shape_functions/ConstantShapeFunction.h"


/**
 * @brief Absolute value of smectic tau parameter (optionally integrated together with a ShapeFunction).
 * @details <p> The wavevector giving maximal tau is selected automatically. The wavevectors compatible with PBC are
 * given by linear combinations with integer coefficients of reciprocal box vectors (Miller indices).
 *
 * <p> The Observable consists of the following values:
 * <ul>
 * <li> (interval value) @a tau - absolute value of the smectic order
 * <li> (interval value) @a tau_k_x, @a tau_k_y, @a tau_k_z - coordinates of the wavevector (present only only if
 * `dumpTauVector_ = true`, see SmecticOrder::SmecticOrder)
 * <li> (nominal value) @a tau_hkl - Miller indices (format: @a "h.k.l")
 * </ul>
 */
class SmecticOrder : public Observable {
private:
    bool dumpTauVector{};
    std::string focalPoint{};
    std::complex<double> tau{};
    std::array<int, 3> hkl{0, 0, 0};
    Vector<3> kTauVector{};
    std::array<int, 3> hklRange{};
    std::shared_ptr<ShapeFunction> shapeFunction;

    static auto calculateTau(const std::array<int, 3> &nTau_, const Packing &packing,
                             const std::vector<Vector<3>> &focalPoints, const std::vector<double> &functionValues);

    [[nodiscard]] std::vector<double> calculateFunctionValues(const Packing &packing, const ShapeTraits &traits) const;

public:
    /**
     * @brief Constructs the smectic order parameter.
     * @details Smectic order wavevector has to be compatible with PBC. Thus, it has to be a linear combination with
     * integer coefficients of reciprocal box vectors. Integer coefficients are called Miller indices @a hkl. When
     * computing the observable, values are calculated for various hkl in @a hklRange range and the ones giving the
     * largest tau value are selected.
     * @param hklRange maximal h, k, l values of Miller indices to be scanned
     * @param dumpTauVector_ if @a true, wavevector will be printed in addition to the tau value
     * @param focalPoint focal point (see ShapeGeometry) that will be used to compute the smectic order
     * @param shapeFunction ShapeFunction \f$ f \f$ that will be integrated together with the smectic order:
     * \f$ \tau = \left\langle f_i \exp(i \mathbf{k} \cdot \mathbf{r}_i) \right\rangle_i \f$. By default, it is a
     * constant equal 1.
     */
    explicit SmecticOrder(const std::array<std::size_t, 3> &hklRange = {5, 5, 5}, bool dumpTauVector_ = false,
                          std::string focalPoint = "o",
                          std::shared_ptr<ShapeFunction> shapeFunction = std::make_shared<ConstantShapeFunction>());

    void calculate(const Packing &packing, double temperature, double pressure,
                   const ShapeTraits &shapeTraits) override;

    [[nodiscard]] std::vector<std::string> getIntervalHeader() const override;
    [[nodiscard]] std::vector<std::string> getNominalHeader() const override;
    [[nodiscard]] std::vector<double> getIntervalValues() const override;
    [[nodiscard]] std::vector<std::string> getNominalValues() const override;
    [[nodiscard]] std::string getName() const override;
};


#endif //RAMPACK_SMECTICORDER_H
