/*
 * ExponentialDecayRegression.cpp
 *
 *  Created on: Mar 27, 2022
 *      Author: ciesla
 */

#include "ExponentialDecayRegression.h"
#include "LinearRegression.h"

#include <algorithm>
#include <cmath>
#include <iostream>


ExponentialDecayRegression::ExponentialDecayRegression(const std::vector<Vector<2>> &points) {
    for (const auto &point: points)
        this->addPoint(point[0], point[1]);

    this->recalculate();
}

Quantity ExponentialDecayRegression::getDecayTime() const {
    Quantity res;
    res.value = this->decay;
    res.error = this->sDecay;
    return res;
}

/**
 * Add data point
 * @param x
 * @param y
 * @param sigma
 */
void ExponentialDecayRegression::addPoint(double x, double y) {
    this->data.emplace_back(x, y);
}

void::ExponentialDecayRegression::recalculate() {
    Expects(this->size() >= 4);
    
    std::sort(this->data.begin(), this->data.end());
    double sum{};
    for (std::size_t i = 1; i < this->data.size(); i++)
        sum += (this->data[i-1].y) * (this->data[i].x - this->data[i-1].x);

    LinearRegression lr;
    for (std::size_t i{}; i < this->data.size() - 1; i++) {
        lr.addPoint(this->data[i].y, sum);
        sum -= (this->data[i].y) * (this->data[i+1].x - this->data[i].x);
    }
    lr.recalculate();
    auto slope = lr.getSlope();
    this->decay = slope.value;
    this->sDecay = slope.error;
}

