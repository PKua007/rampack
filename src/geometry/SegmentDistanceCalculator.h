//
// Created by Piotr Kubala on 27/04/2021.
//

#ifndef RAMPACK_SEGMENTDISTANCECALCULATOR_H
#define RAMPACK_SEGMENTDISTANCECALCULATOR_H

#include "geometry/Vector.h"

class SegmentDistanceCalculator {
private:
    static constexpr double EPSILON = 0.0000000001;

public:
    // Based on
    // Copyright 2001 softSurfer, 2012 Dan Sunday
    // This code may be freely used, distributed and modified for any purpose
    // providing that this copyright notice is included with it.
    // SoftSurfer makes no warranty for this code, and cannot be held
    // liable for any real or imagined damage resulting from its use.
    // Users of this code must verify correctness for their application.
    static double calculate(const Vector<3> &s11, const Vector<3> &s12, const Vector<3> &s21,
                            const Vector<3> &s22)
    {
        Vector<3> u = s12 - s11;
        Vector<3> v = s22 - s21;
        Vector<3> w = s11 - s21;
        double a = u * u, b = u * v, c = v * v, d = u * w, e = v * w;

        double D = a * c - b * b;        // always >= 0
        double sc, sN, sD = D;       // sc = sN / sD, default sD = D >= 0
        double tc, tN, tD = D;       // tc = tN / tD, default tD = D >= 0

        // compute the line parameters of the two closest points
        if (D < EPSILON) { // the lines are almost parallel
            sN = 0.0;      // force using point P0 on segment S1
            sD = 1.0;      // to prevent possible division by 0.0 later
            tN = e;
            tD = c;
        } else {           // get the closest points on the infinite lines
            sN = (b * e - c * d);
            tN = (a * e - b * d);
            if (sN < 0.0) { // sc < 0 => the s=0 edge is visible
                sN = 0.0;
                tN = e;
                tD = c;
            } else if (sN > sD) {  // sc > 1  => the s=1 edge is visible
                sN = sD;
                tN = e + b;
                tD = c;
            }
        }

        if (tN < 0.0) { // tc < 0 => the t=0 edge is visible
            tN = 0.0;
            // recompute sc for this edge
            if (-d < 0.0)
                sN = 0.0;
            else if (-d > a)
                sN = sD;
            else {
                sN = -d;
                sD = a;
            }
        } else if (tN > tD) { // tc > 1  => the t=1 edge is visible
            tN = tD;
            // recompute sc for this edge
            if ((-d + b) < 0.0)
                sN = 0;
            else if ((-d + b) > a)
                sN = sD;
            else {
                sN = (-d + b);
                sD = a;
            }
        }
        // finally do the division to get sc and tc
        sc = (std::abs(sN) < EPSILON ? 0.0 : sN / sD);
        tc = (std::abs(tN) < EPSILON ? 0.0 : tN / tD);

        // get the difference of the two closest points
        Vector<3> dP = w + (sc * u) - (tc * v);  // =  S1(sc) - S2(tc)

        return dP.norm2();   // return the closest distance
    }
};


#endif //RAMPACK_SEGMENTDISTANCECALCULATOR_H
