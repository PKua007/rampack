//
// Created by ciesla on 8/9/22.
//

#ifndef RAMPACK_SMOOTHWEDGETRAITS_H
#define RAMPACK_SMOOTHWEDGETRAITS_H

#include "XenoCollideTraits.h"


/**
 * @brief Class representing a smooth wedge - a convex hull of two spheres with different radii.
 */
class SmoothWedgeTraits : public XenoCollideTraits<SmoothWedgeTraits> {
public:
    /**
     * @brief Class representing the geometry of the wedge (see template parameter of XenoCollide)
     */
    class CollideGeometry {
    private:
        double R{};
        double r{};
        double l{};
        double Rminusr{};
        double Rpos{};
        double rpos{};
        double circumsphereRadius{};
        double insphereRadius{};

        friend SmoothWedgeTraits;

    public:
        /**
         * @brief Creates the wedge along the z axis, with a sphere of radius @a R at the bottom and a sphere of radius
         * @a r at the top, distance by l.
         * @details They are placed along z axis in such a way, that the point {0, 0, 0} is the center of the best
         * (smallest) circumscribed sphere. Namely, bottom sphere is placed in {0, 0, (@a R - @a r - @a l)/2} and the
         * top one in {0, 0, (@a R - @a r + @a l)/2}.
         */
        CollideGeometry(double R, double r, double l);

        [[nodiscard]] Vector<3> getCenter() const { return {}; }

        [[nodiscard]] Vector<3> getSupportPoint(const Vector<3> &n) const {
            Vector<3> nNorm = n.normalized();
            if (this->Rminusr > nNorm[2]*this->l)
                return this->R * nNorm + Vector<3>{0, 0, this->Rpos};
            else
                return this->r * nNorm + Vector<3>{0, 0, this->rpos};
        }

        [[nodiscard]] double getCircumsphereRadius() const { return this->circumsphereRadius; }
        [[nodiscard]] double getInsphereRadius() const { return this->insphereRadius; }
    };

private:
    double R{};
    double r{};
    double l{};
    std::vector<CollideGeometry> shapeModel;
    std::vector<Vector<3>> interactionCentres;

    static double getVolume(double R, double r, double l);

    [[nodiscard]] std::vector<double> calculateRelativeSpherePositions(std::size_t subdivisions) const;

public:
    /**
     * @brief Creates a wedge with parameters @a R, @a r and @a l (see CollideGeometry::CollideGeometry).
     * @details If @a subdivision is at least two, the wedge is divided into that many parts (with equal circumscribed
     * spheres' radii) to lower the number of neighbours in the neighbour grid.
     */
    SmoothWedgeTraits(double R, double r, double l, std::size_t subdivisions = 0);

    /**
     * @brief Returns CollideGeometry object for the interaction center with index @a idx (see XenoCollideTraits).
     */
    [[nodiscard]] const CollideGeometry &getCollideGeometry([[maybe_unused]] std::size_t idx = 0) const {
        return this->shapeModel[idx];
    }

    [[nodiscard]] std::vector<Vector<3>> getInteractionCentres() const override { return this->interactionCentres; }

};


#endif //RAMPACK_SMOOTHWEDGETRAITS_H
