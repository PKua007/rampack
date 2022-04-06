/*
 * LinearRegression.cpp
 *
 *  Created on: 22.06.2017
 *      Author: ciesla
 */

#include "LinearRegression.h"

#include <algorithm>
#include <cmath>

LinearRegression::LinearRegression() {
	this->s = 0.0;
	this->sx = 0.0;
	this->sy = 0.0;
	this->sxx = 0.0;
	this->syy = 0.0;
	this->sxy = 0.0;
	this->a = 0.0;
	this->b = 0.0;
	this->r = 0.0;
	this->sigmaa = 0.0;
	this->sigmab = 0.0;
	this->sigmay = 0.0;
	this->delta = 0.0;
}

LinearRegression::~LinearRegression() {
    this->clear();
}

LinearRegression::LinearRegression(const std::vector<Vector<2>> &points){
    for (Vector<2> data: points){
        this->addXY(data[0], data[1]);
    }
    this->calculate();
}

Quantity LinearRegression::getIntercept() const{
    Quantity res;
    res.value = this->b;
    res.error = this->sigmab;
    return data;
}

Quantity LinearRegression::getSlope() const{
    Quantity res;
    res.value = this->a;
    res.error = this->sigmaa;
    return res;
}

double LinearRegression::getR() const{
    return this->r;
}


void LinearRegression::clear(){
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
void LinearRegression::addXY(double x, double y, double sigma){
	DataElement* d = new DataElement();
	d->x = x;
	d->y = y;
	d->sigma = sigma;
	this->data.push_back(d);
}

void LinearRegression::addXY(double x, double y){
	this->addXY(x, y, 1.0);
}

/**
 * calculates fit values
 */
void LinearRegression::calculate(unsigned int from, unsigned int to){
		this->s = 0.0;
		this->sx = 0.0;
		this->sy = 0.0;
		this->sxx = 0.0;
		this->syy = 0.0;
		this->sxy = 0.0;
		double var;
		to = (to < this->data.size()) ? to : this->data.size();
		for (unsigned int i=from; i<to; i++){
			DataElement *d = this->data[i];
			var = d->sigma*d->sigma;
			this->s += 1/var;
			this->sx += d->x/var;
			this->sy += d->y/var;
			this->sxx += d->x*d->x/var;
			this->syy += d->y*d->y/var;
			this->sxy += d->x*d->y/var;
		}
		this->delta = this->s*this->sxx - this->sx*this->sx;
		this->a = (this->s*this->sxy - this->sx*this->sy)/this->delta;
		this->b = (this->sxx*this->sy - this->sx*this->sxy)/this->delta;

		this->sigmay = 0.0;
		for(unsigned int i=from; i<to; i++){
			DataElement *d = this->data[i];
			this->sigmay += pow(d->y - this->a*d->x - b, 2.0);
		}
		this->sigmay /= (to-from-2);
		this->sigmaa = sqrt(this->sigmay*this->s / this->delta);
		this->sigmab = sqrt(this->sigmay*this->sxx / this->delta);

//		this.sigmay = this.syy + this.a*this.a*this.sxx + this.b*this.b - 2*this.a*this.sxy - 2*this.b*this.sy + 2*this.a*this.b*this.sx;
//		this.sigmaa = this.s/(this.s-2) * this.sigmay/this.delta;
//		this.sigmab = this.sigmaa*this.sxx / this.s;
		this->r = (this->s*this->sxy - this->sx*this->sy) / sqrt((this->s*this->sxx-this->sx*this->sx)*(this->s*this->syy-this->sy*this->sy));
	}

/**
 * calculates fit values
 */
void LinearRegression::calculate(){
	this->calculate(0, this->data.size());
}

/**
 * @param multiplier
 */
void LinearRegression::removeDistantPoints(double multiplier){
	double sigma = this->sigmay/this->data.size();
	for (DataElement *d : this->data) {
		if (pow(d->y - this->a*d->x - this->b, 2.0) > multiplier*sigma){
			this->data.erase(std::remove(this->data.begin(), this->data.end(), d), this->data.end());
		}
	}
	this->calculate();
}


/**
 * @return parameter A from y = Ax + B
 */
double LinearRegression::getA(){
	return this->a;
}

/**
 * @return standard deviation squared for parameter A from y = Ax + B
 */
double LinearRegression::getSA(){
	return this->sigmaa;
}

/**
 * @return parameter B from y = Ax + B
 */
double LinearRegression::getB(){
	return this->b;
}

/**
 * @return standard deviation sqared for parameter B from y = Ax + B
 */
double LinearRegression::getSB(){
	return this->sigmab;
}

/**
 * @return total fit error
 */
double LinearRegression::getSigma(){
	return this->sigmay;
}

/**
 * @return covariance between x and y
 */
double LinearRegression::getR(){
	return this->r;
}

int LinearRegression::size(){
	return this->data.size();
}

/**
 * Tester
 * @param args
 */
/*
void main(int argc, char **argv){
	double x, y;
	LinearRegression lr = new LinearRegression();
	for (int i=0; i<100; i++){
		x = i;
		y = (2+0.5*(Math.random()-0.5))*x + 5  + (Math.random()-0.5);
		System.out.println(x + " " + y);
		lr.addXY(x, y);
	}
	lr.calculate();
	System.out.println(
			String.valueOf(lr.getA()) + "\t" + //$NON-NLS-1$
			String.valueOf(lr.getSA()) + "\t" + //$NON-NLS-1$
			String.valueOf(lr.getB()) + "\t" + //$NON-NLS-1$
			String.valueOf(lr.getSB()) + "\t" + //$NON-NLS-1$
			String.valueOf(lr.getR())
			);
	}
	delete lr;
}
*/
