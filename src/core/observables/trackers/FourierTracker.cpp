//
// Created by pkua on 20.10.22.
//

#include <algorithm>
#include <iterator>

#include "FourierTracker.h"
#include "utils/Assertions.h"


std::string FourierTracker::getModeName() const {
    return this->functionName + "_fourier";
}


void FourierTracker::calculateOrigin(const Packing &packing, const ShapeTraits &shapeTraits) {
    FourierValues fourierValues = this->calculateFourierValues(packing, shapeTraits);

    for (std::size_t idx0{}; idx0 < 3; idx0++) {
        std::size_t wavenumber = this->wavenumbers[idx0];
        double &posComp = this->originPos[idx0];
        if (wavenumber == 0) {
            posComp = 0;
            continue;
        }

        std::size_t idx1 = (idx0 + 1) % 3;
        std::size_t idx2 = (idx0 + 2) % 3;

        std::vector<std::tuple<std::size_t, std::size_t, double>> valuesAlternatives;
        for (std::size_t j{}; j < this->fourierFunctions[idx1].size(); j++) {
            for (std::size_t k{}; k < this->fourierFunctions[idx2].size(); k++) {
                std::array<std::size_t, 3> idx{};
                idx[idx1] = j;
                idx[idx2] = k;

                idx[idx0] = 0;
                double cosVal = fourierValues[idx[0]][idx[1]][idx[2]];
                idx[idx0] = 1;
                double sinVal = fourierValues[idx[0]][idx[1]][idx[2]];

                valuesAlternatives.emplace_back(j, k, cosVal*cosVal + sinVal*sinVal);
            }
        }

        auto comp = [](const auto &elem1, const auto &elem2) {
            return std::get<2>(elem1) < std::get<2>(elem2);
        };
        auto[j, k, val] = *std::max_element(valuesAlternatives.begin(), valuesAlternatives.end(), comp);

        std::array<std::size_t, 3> idx{};
        idx[idx1] = j;
        idx[idx2] = k;

        idx[idx0] = 0;
        double cosVal = fourierValues[idx[0]][idx[1]][idx[2]];
        idx[idx0] = 1;
        double sinVal = fourierValues[idx[0]][idx[1]][idx[2]];
        double angle = std::atan2(cosVal, sinVal);
        posComp = std::fmod(angle + 2*M_PI, 2*M_PI);
    }
}

FourierTracker::FourierValues FourierTracker::calculateFourierValues(const Packing &packing,
                                                                     const ShapeTraits &shapeTraits) const
{
    FourierValues fourierValues{};

    for (std::size_t i{}; i < this->fourierFunctions[0].size(); i++) {
        for (std::size_t j{}; j < this->fourierFunctions[1].size(); j++) {
            for (std::size_t k{}; k < this->fourierFunctions[2].size(); k++) {
                const auto &f0 = this->fourierFunctions[0][i];
                const auto &f1 = this->fourierFunctions[1][j];
                const auto &f2 = this->fourierFunctions[2][k];
                auto &fourierValue = fourierValues[i][j][k];

                auto fProduct = [&f0, &f1, &f2](const Vector<3> &pos) {
                    return f0(pos[0]) * f1(pos[1]) * f2(pos[2]);
                };

                fourierValue = 0;
                for (std::size_t shapeI{}; shapeI < packing.size(); shapeI++) {
                    const auto &shape = packing[shapeI];
                    double functionMapping = this->function(shape, shapeTraits);
                    Vector<3> relativePos = packing.getBox().absoluteToRelative(shape.getPosition());
                    fourierValue += fProduct(relativePos) * functionMapping;
                }
            }
        }
    }

    return fourierValues;
}

FourierTracker::FourierTracker(const std::array<std::size_t, 3> &wavenumber, Function function,
                               std::string functionName)
        : wavenumbers{wavenumber}, function{std::move(function)}, functionName{std::move(functionName)}
{
    Expects(std::any_of(wavenumber.begin(), wavenumber.end(), [](auto item) { return item > 0; }));

    auto functionFiller = [](std::size_t wavenumber) -> std::vector<FourierFunction> {
        if (wavenumber == 0) {
            FourierFunction constant = [](double) -> double { return 1; };
            return {constant};
        } else {
            double k = 2*M_PI*static_cast<double>(wavenumber);
            FourierFunction sin = [k](double xi) { return std::sin(k*xi); };
            FourierFunction cos = [k](double xi) { return std::cos(k*xi); };
            return {cos, sin};
        }
    };

    std::transform(this->wavenumbers.begin(), this->wavenumbers.end(), this->fourierFunctions.begin(), functionFiller);
}
