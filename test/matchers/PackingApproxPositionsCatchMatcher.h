//
// Created by Piotr Kubala on 13/12/2020.
//

#ifndef RAMPACK_PACKINGAPPROXPOSITIONSCATCHMATCHER_H
#define RAMPACK_PACKINGAPPROXPOSITIONSCATCHMATCHER_H

#include <catch2/catch.hpp>
#include <sstream>
#include <vector>
#include <iterator>

#include "core/Packing.h"
#include "geometry/Vector.h"

class PackingApproxPositionsCatchMatcher : public Catch::MatcherBase<Packing> {
private:
    std::vector<Vector<3>> expected;
    double epsilon;

public:
    PackingApproxPositionsCatchMatcher(std::vector<Vector<3>> expected, double epsilon)
            : expected(std::move(expected)), epsilon(epsilon)
    { }

    bool match(const Packing &actual) const override {
        if (this->expected.size() != actual.size())
            return false;
        return std::equal(actual.begin(), actual.end(), this->expected.begin(),
                          [this](const auto &shape, const auto &position) {
                              return shape.getPosition()[0] == Approx(position[0]).epsilon(this->epsilon)
                                     && shape.getPosition()[1] == Approx(position[1]).epsilon(this->epsilon)
                                     && shape.getPosition()[2] == Approx(position[2]).epsilon(this->epsilon);
                          });
    }

    [[nodiscard]] std::string describe() const override {
        std::ostringstream ss;
        ss << "has, within " << this->epsilon << " tolerance threshold, particles" << std::endl;
        for (const auto &position : this->expected) {
            ss << "{" << position[0] << ", " << position[1] << ", " << position[2] << "}, ";
        }
        return ss.str();
    }
};

inline PackingApproxPositionsCatchMatcher
HasParticlesWithApproxPositions(const std::vector<Vector<3>> &expected, double epsilon)
{
    return PackingApproxPositionsCatchMatcher(expected, epsilon);
}

#endif //RAMPACK_PACKINGAPPROXPOSITIONSCATCHMATCHER_H
