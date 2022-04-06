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

ExponentialDecayRegression::ExponentialDecayRegression() {
}

ExponentialDecayRegression::~ExponentialDecayRegression() {
	this->clear();
}

void ExponentialDecayRegression::clear(){
	for(DataElement *d: this->data)
		delete d;
	this->data.clear();
}

/**
 * Add data point
 * @param x
 * @param y
 * @param sigma
 */
void ExponentialDecayRegression::addXY(double x, double y){
	DataElement* d = new DataElement();
	d->x = x;
	d->y = y;
	this->data.push_back(d);
}

void::ExponentialDecayRegression::calculate(){
	std::sort(this->data.begin(), this->data.end());
	double sum = 0;
	for(size_t i=1; i<this->data.size(); i++){
		sum += (this->data[i-1]->y) * (this->data[i]->x - this->data[i-1]->x);
	}

	LinearRegression lr;
	for(size_t i=0; i<this->data.size()-1; i++){
		lr.addXY(this->data[i]->y, sum);
		sum -= (this->data[i]->y)*(this->data[i+1]->x - this->data[i]->x);
	}
	lr.calculate();
	this->decay = lr.getA();
	this->Sdecay = lr.getSA();
}

