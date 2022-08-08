/*
 * ExponentialDecayRegression.h
 *
 *  Created on: Mar 27, 2022
 *      Author: ciesla
 */

#ifndef EXPONENTIALDECAYREGRESSION_H_
#define EXPONENTIALDECAYREGRESSION_H_

#include <vector>
#include "Vector.h"
#include "utils/Quantity.h"


class ExponentialDecayRegression {
private:
    class DataPoint {
    public:
        DataPoint() = default;
        DataPoint(double x, double y) : x{x}, y{y} { }

        double x{};
        double y{};

        friend bool operator<(const DataPoint &d1, const DataPoint &d2) { return d1.x < d2.x; }
    };

	std::vector<DataPoint> data;

public:
	double decay{};
	double sDecay{};
	double initial{};

	ExponentialDecayRegression() = default;
    explicit ExponentialDecayRegression(const std::vector<Vector<2>> &points);

    [[nodiscard]] Quantity getDecayTime() const;

	void clear() { this->data.clear(); }
    [[nodiscard]] std::size_t size() const { return this->data.size(); }

	void addPoint(double x, double y);
	void recalculate();
};

#endif /* EXPONENTIALDECAYREGRESSION_H_ */
