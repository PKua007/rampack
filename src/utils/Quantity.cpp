//
// Created by Piotr Kubala on 12/12/2020.
//

#include <iomanip>
#include <vector>
#include <numeric>
#include <cmath>
#include <sstream>

#include "Quantity.h"

namespace {
    /* Rounds a value to a specific order, ex. 1.254 -> 1.3 if order = -1 or 1233.435 -> 1200 if order = 2 */
    std::string format_up_to_order(double value, int roundOrder) {
        if (roundOrder > 0) {
            double shiftingPow = std::pow(10, roundOrder);
            int result = static_cast<int>(std::round(value / shiftingPow) * shiftingPow);
            return std::to_string(result);
        } else {
            std::ostringstream result;
            result << std::fixed << std::setprecision(-roundOrder) << value;
            return result.str();
        }
    }

    /* Finds the order of a value, ex. 123.34 -> 2, 1.123 -> 1, 0.0345 -> -2 */
    int find_start_order(double value) {
        return static_cast<int>(std::floor(std::log10(std::abs(value))));
    }

    /* Finds the order of a last significant digit, based on passed significantDigits.
     * Important! Note, that order may depend of number of significant digits, ex. 1.0e-1 and 9.98e-2 is the same number
     * - 0.09987 - rounded to, respectively, 2 and 3 significant digits */
    int find_end_order_after_rounding(double value, int significantDigits) {
        int endOrder = find_start_order(value) - significantDigits + 1;

        // Recalculate order, because rounding can change first significant digit position!
        std::string errorRounded = format_up_to_order(value, endOrder);
        return find_start_order(std::stod(errorRounded)) - significantDigits + 1;
    }

}

void Quantity::calculateFromSamples(const std::vector<double> &samples)
{
    if (samples.empty()){
        this->value = 0;
        this->error = 0;
    }
    else if (samples.size() == 1){
        this->value = samples[0];
        this->error = 0;
    }else{
        this->value = std::accumulate(samples.begin(), samples.end(), 0.0) / samples.size();

        auto squareDev = [&](double s, double next) { return s + std::pow(this->value - next, 2); } ;
        this->error = std::accumulate(samples.begin(), samples.end(), 0.0, squareDev);
        this->error = this->error / samples.size() / (samples.size() - 1);
        this->error = std::sqrt(this->error);
    }
}

std::ostream &operator<<(std::ostream &stream, const Quantity &quantity)
{
    if (!quantity.significantDigitsBasedOnError || quantity.error == 0) {
        std::stringstream out;      // isolate stream std::fixed and std::scientific flags
        out << quantity.value << quantity.getSeparatorString() << std::abs(quantity.error);
        stream << out.rdbuf();
    } else {
        int errorEndOrder = find_end_order_after_rounding(quantity.error, quantity.errorDigits);
        int valueEndOrder = errorEndOrder;

        // If error is bigger than value, value will have one significant digit
        int valueStartOrder = find_start_order(quantity.value);
        if (errorEndOrder > valueStartOrder)
            valueEndOrder = find_end_order_after_rounding(quantity.value, 1);

        stream << format_up_to_order(quantity.value, valueEndOrder) << quantity.getSeparatorString();
        stream << format_up_to_order(std::abs(quantity.error), errorEndOrder);
    }

    return stream;
}

std::string Quantity::getSeparatorString() const {
    switch (this->separator) {
        case TABULATOR:
            return "\t";
        case SPACE:
            return " ";
        case PLUS_MINUS:
            return " +- ";
    }
    return "";
}