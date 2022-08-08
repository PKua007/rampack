/*
 * LinearRegression.h
 *
 *  Created on: 22.06.2017
 *      Author: ciesla
 */

#ifndef LINEARREGRESSION_H_
#define LINEARREGRESSION_H_

#include <vector>
#include "Vector.h"
#include "utils/Quantity.h"
#include "utils/Assertions.h"


class LinearRegression {
private:
    class DataPoint {
    public:
        DataPoint() = default;
        DataPoint(double x, double y, double sigma) : x{x}, y{y}, sigma{sigma} { }

        double x{};
        double y{};
        double sigma{};
    };

    std::vector<DataPoint> data;

    double a{}, b{}, sigmaY{}, sigmaA{}, sigmaB{}, r{};

public:
    LinearRegression() = default;
    explicit LinearRegression(const std::vector<Vector<2>> &points);

    [[nodiscard]] Quantity getIntercept() const;
    [[nodiscard]] Quantity getSlope() const;
    [[nodiscard]] double getR() const { return this->r; }

    void clear() { this->data.clear(); }
    [[nodiscard]] std::size_t size() const { return this->data.size(); }

    void addPoint(double x, double y, double sigma);
    void addPoint(double x, double y);
    void recalculate(std::size_t fromIdx, std::size_t toIdx);
    void recalculate() { this->recalculate(0, this->data.size()); }
};

#endif /* LINEARREGRESSION_H_ */
