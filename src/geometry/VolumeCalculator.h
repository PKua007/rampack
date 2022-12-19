//
// Created by Piotr Kubala on 19/12/2022.
//

#ifndef RAMPACK_VOLUMECALCULATOR_H
#define RAMPACK_VOLUMECALCULATOR_H


class VolumeCalculator {
public:
    static double sphericalCap(double radius, double capHeight);
    static double sphereIntersection(double radius1, double radius2, double distance);
};


#endif //RAMPACK_VOLUMECALCULATOR_H
