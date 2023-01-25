//
// Created by ciesla on 12/30/22.
//

#include <omp.h>
#include "EVBox.h"

std::uniform_real_distribution<double> EVBox::u01Distribution(0,1);
std::mt19937 EVBox::mt;
FreeBoundaryConditions EVBox::fbc;


EVBox::EVBox(const Vector<3> v, double l) : position{v},
                                            length{l}
    {
    }

void EVBox::divide(std::vector<EVBox *> &newBoxes) const{
        EVBox *box;

        newBoxes.clear();
        double newLength = 0.5*this->length;
        for(ushort ix = 0; ix < 2; ix++) {
            for (ushort iy = 0; iy < 2; iy++) {
                for (ushort iz = 0; iz < 2; iz++) {
                    Vector<3> translation({newLength * ix, newLength * iy, newLength * iz});
                    box = new EVBox(position + translation, newLength);
                    newBoxes.push_back(box);
                }
            }
        }
    }

    void EVBox::sampleCorners(const Shape& originShape, Shape testShape, const Interaction &interaction){
        for (ushort ix = 0; ix < 2; ix++) {
            for (ushort iy = 0; iy < 2; iy++) {
                for (ushort iz = 0; iz < 2; iz++) {
                    Vector<3> translation({this->length * ix, this->length * iy, this->length * iz});
                    testShape.setPosition(this->position + translation);
                    if (interaction.overlapBetweenShapes(originShape, testShape, this->fbc))
                        this->intersections++;
                    this->samples++;
                }
            }
        };
    }


    void EVBox::sampleMC(Matrix<3, 3, double> *orientation, const Shape& originShape, const Shape& testShape, const Interaction &interaction, size_t samples){
        size_t threads = omp_get_max_threads();
        size_t *counter = new size_t[threads];
        Shape *testShapes = new Shape[threads];
        for (size_t i=0; i<threads; i++){
            counter[i] = 0;
            testShapes[i] = testShape;
            if (orientation != nullptr)
                testShapes[i].setOrientation(*orientation);
        }

#pragma omp parallel for schedule(dynamic)
        for(size_t i=0; i<samples; i++) {
            size_t threadId = omp_get_thread_num();
            Vector<3> translation{EVBox::u01Distribution(this->mt)*this->length,
                                  EVBox::u01Distribution(this->mt)*this->length,
                                  EVBox::u01Distribution(this->mt)*this->length};
            testShapes[threadId].setPosition(this->position + translation);
            if (orientation == nullptr){
                testShapes[threadId].setOrientation(Matrix<3, 3>::rotation(
                        2 * M_PI * this->u01Distribution(this->mt),
                        std::asin(2 * this->u01Distribution(this->mt) - 1),
                        2 * M_PI * this->u01Distribution(this->mt)));
            }
            if (interaction.overlapBetweenShapes(originShape, testShapes[threadId], EVBox::fbc)) {
                counter[threadId]++;
            }
        }
        this->samples += samples;
        this->intersections += std::accumulate(counter, counter+threads, 0);

        delete[] counter;
        delete[] testShapes;
    }

    const Vector<3>& EVBox::getPosition() const{
        return this->position;
    }

    double EVBox::volume() const{
        return this->length*this->length*this->length;
    }

    size_t EVBox::getIntersections() const{
        return this->intersections;
    }

    size_t EVBox::getSamples() const{
        return this->samples;
    }

    double EVBox::getCoverage() const{
        return static_cast<double>(this->intersections)/static_cast<double>(this->samples);
    }
