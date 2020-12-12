//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_QUANTITY_H
#define RAMPACK_QUANTITY_H


#include <vector>
#include <iosfwd>

#include "Assertions.h"

/**
 * @brief A simple class representing physical quantity with error.
 *
 * The class also includes simple print routines, where the number of significant digits can be based on error.
 */
class Quantity {

public:
    /**
     * @brief Enumeration representing what sould be used to separate value from error when printing.
     */
    enum Separator {
        TABULATOR,
        SPACE,
        PLUS_MINUS
    };

    /**
     * @brief The value of a quantity.
     */
    double value = 0;

    /**
     * @brief The error of a quantiy.
     */
    double error = 0;

    /**
     * @brief What should be used to separate value from error when printing.
     */
    Separator separator = TABULATOR;

    /**
     * @brief It set true, the number of significant digits will be based on error. If false, default double printing.
     */
    bool significantDigitsBasedOnError = true;

    Quantity() = default;
    Quantity(double value, double error, int errorDigits = 4) : value(value), error(error), errorDigits(errorDigits) {
        Expects(errorDigits > 0 && errorDigits < 16);
    }

    /**
     * @brief Creates the quantity as a mean of samples, with estimated error of the mean.
     * @param samples A vector of doubles to calculate the quantity from
     * @return the quantity from samples
     */
    void calculateFromSamples(const std::vector<double> &samples);

    /**
     * @brief Prints the quantity on given @a std::ostream The behaviour can be manipulated via Quantity::separator and
     * Quantity::significantDigitsBasedOnError fields.
     * @param stream the @a std::ostream to print onto
     * @param quantity the quantity printed
     * @return the reference to `stream`
     */
    friend std::ostream &operator<<(std::ostream &stream, const Quantity &quantity);

private:
    int errorDigits = 4;

    [[nodiscard]] std::string getSeparatorString() const;
};


#endif //RAMPACK_QUANTITY_H
