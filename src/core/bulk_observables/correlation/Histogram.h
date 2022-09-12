//
// Created by pkua on 12.09.22.
//

#ifndef RAMPACK_HISTOGRAM_H
#define RAMPACK_HISTOGRAM_H

#include <ostream>


class Histogram {
public:
    void addToBin(double value, std::size_t binIdx);
    void nextSnapshot();
    void print(std::ostream &out) const;
    void clear();
};


#endif //RAMPACK_HISTOGRAM_H
