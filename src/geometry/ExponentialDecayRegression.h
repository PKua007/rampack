/*
 * ExponentialDecayRegression.h
 *
 *  Created on: Mar 27, 2022
 *      Author: ciesla
 */

#ifndef EXPONENTIALDECAYREGRESSION_H_
#define EXPONENTIALDECAYREGRESSION_H_

#include <vector>
#include <utility>

class ExponentialDecayRegression {

	class DataElement{
	public:
		double x, y;

		bool operator< (const DataElement& d){
		        return (this->x < d.x);
		}
	};

private:
	std::vector<DataElement *> data;

public:
	double decay;
	double Sdecay;
	double initial;

	ExponentialDecayRegression();
	~ExponentialDecayRegression();
	void clear();
	void addXY(double x, double y);
	void calculate();
	double getDecayTime();
	double getInitialValue();
	int size();

};

#endif /* EXPONENTIALDECAYREGRESSION_H_ */
