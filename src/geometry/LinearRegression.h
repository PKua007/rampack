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

class LinearRegression {

	class DataElement{
	public:
		double x, y, sigma;
	};

private:


	std::vector<DataElement *> data;

	double s, sx, sy, sxx, sxy, syy, delta;
	double a, b, sigmay, sigmaa, sigmab, r;

public:
	LinearRegression();
    explicit LinearRegression(const std::vector<Vector<2>> &points);
	virtual ~LinearRegression();

    [[nodiscard]] Quantity getIntercept() const;

    [[nodiscard]] Quantity getSlope() const;

    [[nodiscard]] double LinearRegression::getR() const;



    void clear();
	void addXY(double x, double y, double sigma);
	void addXY(double x, double y);
	void calculate(unsigned int from, unsigned int to);
	void calculate();
	void removeDistantPoints(double multiplier);
	double getA();
	double getSA();
	double getB();
	double getSB();
	double getSigma();
	double getR();
	int size();

};

#endif /* LINEARREGRESSION_H_ */
