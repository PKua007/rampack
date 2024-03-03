//
// Created by Piotr Kubala on 03/03/2024.
//

#ifndef RAMPACK_SHAPEPARAMETERRANDOMIZER_H
#define RAMPACK_SHAPEPARAMETERRANDOMIZER_H

#include <string>
#include <random>


class ShapeParameterRandomizer {
public:
    virtual ~ShapeParameterRandomizer() = default;

    [[nodiscard]] virtual std::string randomize(const std::string &oldValue, std::mt19937 &mt) const = 0;
};


#endif //RAMPACK_SHAPEPARAMETERRANDOMIZER_H
