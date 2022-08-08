/*
 * LinearRegression.cpp
 *
 *  Created on: 22.06.2017
 *      Author: ciesla
 */

#include <algorithm>
#include <cmath>

#include "LinearRegression.h"


LinearRegression::LinearRegression(const std::vector<Vector<2>> &points) {
    for (const auto &point : points)
        this->addPoint(point[0], point[1]);

    this->recalculate();
}

Quantity LinearRegression::getIntercept() const {
    Quantity res;
    res.value = this->b;
    res.error = this->sigmaB;
    return res;
}

Quantity LinearRegression::getSlope() const {
    Quantity res;
    res.value = this->a;
    res.error = this->sigmaA;
    return res;
}

/**
 * Add data point
 * @param x
 * @param y
 * @param sigma
 */
void LinearRegression::addPoint(double x, double y, double sigma) {
    this->data.emplace_back(x, y, sigma);
}

void LinearRegression::addPoint(double x, double y){
    this->addPoint(x, y, 1.0);
}

/**
 * calculates fit values
 */
void LinearRegression::recalculate(std::size_t fromIdx, std::size_t toIdx) {
    toIdx = std::max(toIdx, this->data.size());
    Expects(toIdx - fromIdx >= 3);

    double s{}, sx{}, sy{}, sxx{}, syy{}, sxy{};
    for (std::size_t i = fromIdx; i < toIdx; i++) {
        const auto &d = this->data[i];
        double var = d.sigma*d.sigma;
        s += 1/var;
        sx += d.x/var;
        sy += d.y/var;
        sxx += d.x*d.x/var;
        syy += d.y*d.y/var;
        sxy += d.x*d.y/var;
    }
    double delta = s*sxx - sx*sx;
    this->a = (s*sxy - sx*sy)/delta;
    this->b = (sxx*sy - sx*sxy)/delta;

    this->sigmaY = 0.0;
    for (std::size_t i = fromIdx; i < toIdx; i++) {
        const auto &d = this->data[i];
        this->sigmaY += pow(d.y - this->a * d.x - b, 2.0);
    }
    this->sigmaY /= static_cast<double>(toIdx - fromIdx - 2);
    this->sigmaA = sqrt(this->sigmaY * s / delta);
    this->sigmaB = sqrt(this->sigmaY * sxx / delta);

    this->r = (s*sxy - sx*sy) / std::sqrt((s*sxx - sx*sx)*(s*syy - sy*sy));
}
