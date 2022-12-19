//
// Created by Piotr Kubala on 19/12/2022.
//

#ifndef RAMPACK_VOLUMECALCULATOR_H
#define RAMPACK_VOLUMECALCULATOR_H


/**
 * @brief Set of functions for calculating volumes of various objects.
 */
class VolumeCalculator {
public:
    /**
     * @brief Calculates a volume of a spherical cap with height @a capHeight cut out from a ball of radius @a radius.
     */
    static double sphericalCap(double radius, double capHeight);

    /**
     * @brief Calculates a volume of the union of two overlapping balls with radii @a radius1, @a radius2 distant by
     * distance.
     */
    static double sphereIntersection(double radius1, double radius2, double distance);
};


#endif //RAMPACK_VOLUMECALCULATOR_H
