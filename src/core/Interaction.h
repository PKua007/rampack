//
// Created by Piotr Kubala on 19/12/2020.
//

#ifndef RAMPACK_INTERACTION_H
#define RAMPACK_INTERACTION_H

#include <vector>

#include "Shape.h"
#include "BoundaryConditions.h"

/**
 * @brief A class representing the interaction between molecules.
 * @details The molecule can consist of many interaction centers, possibly interacting in a different way. Interaction
 * can have either of: hard-core and soft term, or both.
 */
class Interaction {
private:
    [[nodiscard]] static Vector<3> getCentrePositionForShape(const Shape &shape, const Vector<3> &centre);

public:
    virtual ~Interaction() = default;

    /**
     * @brief Returns @a true, if the interaction includes a hard-core repulsion.
     */
    [[nodiscard]] virtual bool hasHardPart() const = 0;

    /**
     * @brief Returns @a true, if the interaction includes a soft potential.
     */
    [[nodiscard]] virtual bool hasSoftPart() const = 0;

    /**
     * @brief Returns @a true, if the interaction includes a wall part.
     */
    [[nodiscard]] virtual bool hasWallPart() const = 0;

    /**
     * @brief Returns @a true, if the hard-core shape is convex.
     * @details The presence or the lack of soft part should not affect convexity.
     */
    [[nodiscard]] virtual bool isConvex() const = 0;

    /**
     * @brief Returns the soft interaction energy between two interaction centers of two molecules (for example two
     * Lennard-Jones interaction centres in a complex molecule)
     * @param pos1 position of the first interaction center (not the center of particle)
     * @param orientation1 orientation of the first molecule
     * @param idx1 the index of the first interaction center within a molecule
     * @param pos2 position of the second interaction center (not the center of particle)
     * @param orientation2 orientation of the second molecule
     * @param idx2 the index of the second interaction center within a molecule
     * @param bc boundary conditions used to calculate the interaction
     * @return the soft interaction energy between two interaction centers of two molecules
     */
    [[nodiscard]] virtual double calculateEnergyBetween([[maybe_unused]] const Vector<3> &pos1,
                                                        [[maybe_unused]] const Matrix<3, 3> &orientation1,
                                                        [[maybe_unused]] std::size_t idx1,
                                                        [[maybe_unused]] const Vector<3> &pos2,
                                                        [[maybe_unused]] const Matrix<3, 3> &orientation2,
                                                        [[maybe_unused]] std::size_t idx2,
                                                        [[maybe_unused]] const BoundaryConditions &bc) const
    {
        return 0;
    }

    /**
     * @brief Returns @a true, if two interaction centers of two molecules overlap (for example two spheres from
     * a polysphere shape).
     * @param pos1 position of the first interaction center (not the center of particle)
     * @param orientation1 orientation of the first molecule
     * @param idx1 the index of the first interaction center within a molecule
     * @param pos2 position of the second interaction center (not the center of particle)
     * @param orientation2 orientation of the second molecule
     * @param idx2 the index of the second interaction center within a molecule
     * @param bc boundary conditions used to calculate the interaction
     * @return @a true, if two interaction centers of two molecules overlap
     */
    [[nodiscard]] virtual bool overlapBetween([[maybe_unused]] const Vector<3> &pos1,
                                              [[maybe_unused]] const Matrix<3, 3> &orientation1,
                                              [[maybe_unused]] std::size_t idx1,
                                              [[maybe_unused]] const Vector<3> &pos2,
                                              [[maybe_unused]] const Matrix<3, 3> &orientation2,
                                              [[maybe_unused]] std::size_t idx2,
                                              [[maybe_unused]] const BoundaryConditions &bc) const
    {
        return false;
    }


    /**
     * @brief Returns @a true, if a given interaction center overlaps a wall defined by @a wallOrigin and @a wallVector.
     * @param pos position of the interaction center (not the center of particle)
     * @param orientation orientation of the molecule
     * @param idx the index of the interaction center within a molecule
     * @param wallOrigin arbitrary point lying on the plane of a wall
     * @param wallVector vector normal to the wall (with a unit norm); the direction it points in is in front of a wall,
     * while the opposite direction is behind the wall
     * @return @a true, if any part of the interaction centre lies behind the wall
     */
    [[nodiscard]] virtual bool overlapWithWall([[maybe_unused]] const Vector<3> &pos,
                                               [[maybe_unused]] const Matrix<3, 3> &orientation,
                                               [[maybe_unused]] std::size_t idx,
                                               [[maybe_unused]] const Vector<3> &wallOrigin,
                                               [[maybe_unused]] const Vector<3> &wallVector) const
    {
        return false;
    }

    /**
     * @brief Returns the distance at which either pair of interaction centres ceases to interact (the cut-off
     * distance).
     */
    [[nodiscard]] virtual double getRangeRadius() const { return std::numeric_limits<double>::infinity(); }

    /**
     * @brief Returns a list of positions of interaction centers for a molecule placed in the origin and with a default
     * orientation.
     * @details An empty list means there is a single interaction centre in the origin.
     */
    [[nodiscard]] virtual std::vector<Vector<3>> getInteractionCentres() const { return {}; }

    /**
     * @brief Returns a distance at which two molecules cease to interact (opposed to Interaction::getRangeRadius which
     * applies to a single pair of interaction centers).
     * @details The distance is calculated between molecules centers.
     */
    [[nodiscard]] virtual double getTotalRangeRadius() const;

    /**
     * @brief A helper function, which calculates the energy between two whole molecules - it uses
     * Interaction::calculateEnergyBetween virtual method exhaustively for all pairs of interaction centers of the two
     * given shapes.
     */
    [[nodiscard]] double calculateEnergyBetweenShapes(const Shape &shape1, const Shape &shape2,
                                                      const BoundaryConditions &bc) const;

    /**
     * @brief A helper function, which check if two whole molecules overlap - it uses Interaction::overlapBetween
     * virtual method exhaustively for all pairs of interaction centers of the two given shapes.
     */
    [[nodiscard]] bool overlapBetweenShapes(const Shape &shape1, const Shape &shape2,
                                            const BoundaryConditions &bc) const;


    [[nodiscard]] bool overlapWithWallForShape(const Shape &shape, const Vector<3> &wallOrigin,
                                               const Vector<3> &wallVector) const;
};


#endif //RAMPACK_INTERACTION_H
