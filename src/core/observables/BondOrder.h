//
// Created by pkua on 05.04.2022.
//

#ifndef RAMPACK_BONDORDER_H
#define RAMPACK_BONDORDER_H

#include "core/Observable.h"


/**
 * @brief Parameter measuring local order of nearest neighbour.
 * @details <p> The parameter is calculated with respect to the plane specified in the constructor. For rank @a r, @a r
 * nearest neighbours of each particles are found. Then, the angle \f$ \phi_i \f$ between an arbitrary direction lying
 * in the plane and the projection of vector joining molecule with the i-th neighbour is computed. Local parameter is
 * computed as \f$ \psi_r = 1/r \sum_i \exp(i r \phi_i) \f$. At the end, the absolute value of this quantity is averaged
 * over all molecules in the system.
 *
 * <p> Physically meaningful ranks are 3, 4 and 6 for triatic, tetratic and hexatic order, however the class allows all
 * values greater or equal 2 (because why not).
 */
class BondOrder : public Observable {
private:
    using KnnVector = std::vector<std::pair<std::size_t, double>>;

    std::vector<std::size_t> ranks;
    std::vector<double> psis;
    std::vector<std::string> header;
    Vector<3> millerIndices;

    std::string layeringPointName;
    std::string bondOrderPointName;

    Vector<3> kVector;
    Vector<3> planeVector1;
    Vector<3> planeVector2;
    double tauAngle{};

    static void insertDistance(KnnVector &knnVector, std::size_t particleIdx, double distance2);

    [[nodiscard]] std::vector<KnnVector> constructKnn(const std::vector<Vector<3>> &layeringPoints,
                                                      const std::vector<Vector<3>> &bondOrderPoints,
                                                      const BoundaryConditions &bc);
    void calculateLayerGeometry(const Packing &packing, const std::vector<Vector<3>> &layeringPoints);
    [[nodiscard]] double doCalculateBondOrder(const std::vector<Vector<3>> &bondOrderPoints, std::size_t rank,
                                              const std::vector<KnnVector> &knn, const BoundaryConditions &bc);

public:
    /**
     * @brief Creates the object calculating bond order of a single rank @a rank.
     * @param rank rank compute (usually equal 3, 4 or 6)
     * @param planeMillerIndices parameter specifying the plane in which bond order should be computed. It is given as
     * Miller indices with respect to simulation box.
     * @param layeringPointName named point (see ShapeGeometry::getNamedPoint) which should be used to assign shapes
     * to layers
     * @param bondOrderPointName named point (see ShapeGeometry::getNamedPoint) which should be used to calculate
     * nearest neighbours and bond order
     */
    BondOrder(std::size_t rank, const std::array<int, 3> &planeMillerIndices,
              const std::string &layeringPointName = "cm", const std::string &bondOrderPointName = "cm")
            : BondOrder(std::vector<std::size_t>{rank}, planeMillerIndices, layeringPointName, bondOrderPointName)
    { }

    /**
     * @brief Creates the object calculating bond order of multiple ranks.
     * @param ranks ranks to compute (usually one or more of: 3, 4 and 6)
     * @param planeMillerIndices parameter specifying the plane in which bond order should be computed. It is given as
     * Miller indices with respect to simulation box.
     * @param layeringPointName named point (see ShapeGeometry::getNamedPoint) which should be used to assign shapes
     * to layers
     * @param bondOrderPointName named point (see ShapeGeometry::getNamedPoint) which should be used to calculate
     * nearest neighbours and bond order
     */
    BondOrder(std::vector<std::size_t> ranks, const std::array<int, 3> &planeMillerIndices,
              std::string layeringPointName = "cm", std::string bondOrderPointName = "cm");

    /**
     * @brief Computes the bond order parameter(s) for given @a packing. Rest of parameters are ignored.
     */
    void calculate(const Packing &packing, double temperature, double pressure,
                   const ShapeTraits &shapeTraits) override;

    [[nodiscard]] std::vector<std::string> getIntervalHeader() const override { return this->header; }
    [[nodiscard]] std::vector<std::string> getNominalHeader() const override { return {}; }
    [[nodiscard]] std::vector<double> getIntervalValues() const override { return this->psis; }
    [[nodiscard]] std::vector<std::string> getNominalValues() const override { return {}; }
    [[nodiscard]] std::string getName() const override { return "bond order"; }
};


#endif //RAMPACK_BONDORDER_H
