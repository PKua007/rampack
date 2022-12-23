//
// Created by pkua on 20.10.22.
//

#ifndef RAMPACK_FOURIERTRACKER_H
#define RAMPACK_FOURIERTRACKER_H

#include <array>
#include <functional>
#include <optional>
#include <set>

#include "core/observables/GoldstoneTracker.h"


/**
 * @brief GoldstoneTracker following the maxima of a linear combination of Fourier decomposition functions in 1D, 2D or
 * 3D, with specific, given wavenumbers.
 * @details <p> The class works with a specific function \f$ f(i) \f$ passed in the constructor, which returns some
 * value for i-th shape. Assume, that we choose to perform 2D tracking of \f$ x \f$ and \f$ y \f$ coordinates with
 * (integer) wavenumbers \f$ n_x, n_y \f$. It will then calculate the following sums:
 * \f[
 * \begin{align}
 *     C_{cc} &= \sum_{i=1}^{N} f(i) \cos(n_x \cdot 2\pi \tilde{x}_i)\cos(n_y \cdot 2\pi \tilde{y}_i), \\
 *     C_{cs} &= \sum_{i=1}^{N} f(i) \cos(n_x \cdot 2\pi \tilde{x}_i)\sin(n_y \cdot 2\pi \tilde{y}_i), \\
 *     C_{sc} &= \sum_{i=1}^{N} f(i) \sin(n_x \cdot 2\pi \tilde{x}_i)\cos(n_y \cdot 2\pi \tilde{y}_i), \\
 *     C_{ss} &= \sum_{i=1}^{N} f(i) \sin(n_x \cdot 2\pi \tilde{x}_i)\sin(n_y \cdot 2\pi \tilde{y}_i),
 * \end{align}
 * \f]
 * where \f$ N \f$ is total number of shapes, \f$ i \f$ is the index of i-th shape, and \f$ \tilde{x}, \tilde{y} \f$ are
 * shape coordinates relative to the simulation box. It then calculates the position of the maximum of the following
 * function:
 * \f[
 *     F(\tilde{x}, \tilde{y}) = C_{cc} \cos(n_x \cdot 2\pi \tilde{x}) \cos(n_y \cdot 2\pi \tilde{y})
 *      + C_{cs} \cos(n_x \cdot 2\pi \tilde{x}) \sin(n_y \cdot 2\pi \tilde{y})
 *      + C_{sc} \sin(n_x \cdot 2\pi \tilde{x}) \cos(n_y \cdot 2\pi \tilde{y})
 *      + C_{ss} \sin(n_x \cdot 2\pi \tilde{x}) \sin(n_y \cdot 2\pi \tilde{y})
 * \f]
 * which is then converted to absolute coordinates and reported as the system origin. This function is in essence
 * a Fourier decomposition of \f$ f(i) \rho(r_i) \f$ with all components but ones given by \f$ n_x, n_y \f$ stripped
 * off, where \f$ \rho(r_i) \f$ is phase-space density. The provided description of the procedure for 2D naturally
 * generalizes to 1D or 3D.
 *
 * <p> What this maximum represents depends on the function \f$ f(i) \f$. For example, if we are tracking maxima of
 * 2D density modulation, the function should be just a constant. However, if we are tracking 2D modulation of
 * x component of polarization, the function should return x component of polarization for the given shape.
 *
 * <p> Please note, that in many cases the function \f$ F(\dots) \f$ may have more than one maximum within the
 * simulation box. The class then chooses the one which is closest (according to periodic boundary conditions) to the
 * previously found maximum (in the previous invocation of FourierTracker::calculateOrigin), or the one closest to
 * `(0, 0, 0)` for the first invocation (or after FourierTracker::reset was called).
 */
class FourierTracker : public GoldstoneTracker {
public:
    /**
     * @brief Function \f$ f(i) \f$ as described in the class description, taking Shape and ShapeTraits as arguments and
     * returning a single @a double value.
     */
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
    [[nodiscard]] std::tuple<double, double> findBestSinCosZ(const std::set<double> &roots, double U, double V,
                                                             double W, double X, double Y, double Z) const;

public:
    /**
     * @brief Creates a fourier tracker with given parameters.
     * @param wavenumbers wavenumbers to select from the Fourier decomposition (as described in the class description).
     * One or two wavenumbers can be zero, which means that the computation is restricted to, respectively, 2D or 1D.
     * For example, if `wavenumbers = {2, 0, 0}`, only the oscillations with period 1/2 in x direction will be measures.
     * @param function the function \f$ f(i) \f$, as described in the class description
     * @param functionName the name of function used in FourierTracker::getTrackingMethodName
     */
    FourierTracker(const std::array<std::size_t, 3> &wavenumbers, Function function, std::string functionName);

    /**
     * @brief Returns the method name, which is '`functionName`_tracker', with `functionName` as passed in the
     * constructor.
     * @return
     */
    [[nodiscard]] std::string getTrackingMethodName() const override;

    void calculateOrigin(const Packing &packing, const ShapeTraits &shapeTraits) override;
    void reset() override;
};


#endif //RAMPACK_FOURIERTRACKER_H
