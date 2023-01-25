//
// Created by Michal Ciesla on 10/12/22.
//

#include <iostream>
#include "EVEstimator.h"
#include "EVBox.h"

EVEstimator::EVEstimator(const ShapeTraits &traits)
        : traits{traits},
          interaction{traits.getInteraction()},
          translationDistribution(-traits.getInteraction().getTotalRangeRadius(),
                                  traits.getInteraction().getTotalRangeRadius()),
          u01Distribution(0.0, 1.0)
{

}

void EVEstimator::clear() {
    this->intersectionCounter = 0;
    this->sampleCounter = 0;
}

void EVEstimator::calculateMCResults(double volume){
    double p = (static_cast<double>(this->intersectionCounter))/(static_cast<double>(this->sampleCounter));
    double variance = p*(1-p);
    this->result = p*volume/this->traits.getGeometry().getVolume();
    this->error = std::sqrt(variance/(static_cast<double>(this->sampleCounter)))*volume/this->traits.getGeometry().getVolume();
}

void EVEstimator::calculateBoxResults(std::queue<const EVBox *> *partiallyCoveredBoxes, double coveredVolume){
    double volume = coveredVolume;
    double volumeVariance = 0.0;
    std::queue<const EVBox *> copy = *partiallyCoveredBoxes;
    while (!copy.empty()) {
        const EVBox *box = copy.front();
        copy.pop();
        double p = box->getCoverage();;
        volume += p*(box->volume());
        volumeVariance += p*(1-p)*(box->volume())*(box->volume());
    }
    this->result = volume/this->traits.getGeometry().getVolume();
    this->error = std::sqrt(volumeVariance)/this->traits.getGeometry().getVolume();
}


void EVEstimator::calculateMC(Matrix<3, 3, double>* orientation, size_t samples){
    double range = traits.getInteraction().getTotalRangeRadius();
    EVBox box(Vector<3>({-range, -range, -range}), 2 * range);
    box.sampleMC(orientation, this->origin, this->testShape, this->interaction, samples);
    this->intersectionCounter = box.getIntersections();
    this->sampleCounter = box.getSamples();
    this->calculateMCResults(box.volume());
}

void EVEstimator::calculateMC(Matrix<3, 3, double> *orientation, double expectedError){
    double range = traits.getInteraction().getTotalRangeRadius();
    EVBox box(Vector<3>({-range, -range, -range}), 2 * range);

    std::size_t todoSamples = 1e3;
    do {
        box.sampleMC(orientation, this->origin, this->testShape, this->interaction, todoSamples);
        this->intersectionCounter = box.getIntersections();
        this->sampleCounter = box.getSamples();
        this->calculateMCResults(box.volume());
        // error = v*sigma/sqrt(n)
        double p = (static_cast<double>(this->intersectionCounter))/(static_cast<double>(this->sampleCounter));
        double variance = p*(1-p);
        std::size_t expectedSamples = static_cast<size_t>(std::pow(box.volume() / expectedError, 2.0) * variance);
        if (expectedSamples < this->sampleCounter)
            todoSamples = 1e3;
        else
            todoSamples = expectedSamples - this->sampleCounter;
    }while(this->error > expectedError);
}

double EVEstimator::getResult() const {
    return this->result;
}

double EVEstimator::getError() const {
    return this->error;
}

void EVEstimator::calculateBox(Matrix<3, 3, double>* orientation, double expectedError) {
    const size_t MCSamples = 20; //static_cast<size_t>(1.0 / expectedError);
    double coveredVolume = 0.0;
    double range = traits.getInteraction().getTotalRangeRadius();
    std::queue<const EVBox *> *partiallyCoveredBoxes = new std::queue<const EVBox *>();
    std::vector<EVBox *> dividedBoxes;

    EVBox *initialBox = new EVBox(Vector<3>({-range, -range, -range}), 2 * range);
    initialBox->sampleCorners(this->origin, this->testShape, this->interaction);
    initialBox->sampleMC(orientation, this->origin, this->testShape, this->interaction, MCSamples);
    partiallyCoveredBoxes->push(initialBox);
    this->testShape.setOrientation(*orientation);

    do{
        std::queue<const EVBox *> *tmpQueue = new std::queue<const EVBox *>();
        while (!partiallyCoveredBoxes->empty()){
            const EVBox *box = partiallyCoveredBoxes->front();
            partiallyCoveredBoxes->pop();
            box->divide(dividedBoxes);
            delete box;
            for (EVBox *box: dividedBoxes) {
                box->sampleCorners(this->origin, this->testShape, this->interaction);
                box->sampleMC(orientation, this->origin, this->testShape, this->interaction, MCSamples);
                if (box->getIntersections() == box->getSamples()) {
                    coveredVolume += box->volume();
                } else if (box->getIntersections() > 0) {
                    tmpQueue->push(box);
                }
            }
        }
        delete partiallyCoveredBoxes;
        partiallyCoveredBoxes = tmpQueue;
        this->calculateBoxResults(partiallyCoveredBoxes, coveredVolume);
    }while(this->error/this->result > expectedError);
    this->calculateBoxResults(partiallyCoveredBoxes, coveredVolume);
}