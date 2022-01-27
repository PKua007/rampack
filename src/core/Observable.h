//
// Created by Piotr Kubala on 22/03/2021.
//

#ifndef RAMPACK_OBSERVABLE_H
#define RAMPACK_OBSERVABLE_H

#include <vector>
#include <string>

#include "Packing.h"
#include "ShapeTraits.h"

/**
 * @brief A class representing an observable.
 * @details The observable has its main name and consists of one or mote interval and/or nominal values. Interval
 * values are the ones that can be averaged, while nominal ones cannot.
 */
class Observable {
public:
    virtual ~Observable() = default;

    /**
     * @brief Calculates all observable values for a @a packing at give @a temperature and @a pressure. The packing
     * consists of shapes described by @a shapeTraits.
     * @details The calculated values should be stored internally and accessible later using
     * Observable::getIntervalValues and Observable::getNominalValues methods.
     */
    virtual void calculate(const Packing &packing, double temperature, double pressure,
                           const ShapeTraits &shapeTraits) = 0;

    /**
     * @brief Returns name of all interval values.
     */
    [[nodiscard]] virtual std::vector<std::string> getIntervalHeader() const = 0;

    /**
     * @brief Returns name of all nominal values.
     */
    [[nodiscard]] virtual std::vector<std::string> getNominalHeader() const = 0;

    /**
     * @brief Returns interval values calculated in the last invocation of Observable::calculate method.
     */
    [[nodiscard]] virtual std::vector<double> getIntervalValues() const = 0;

    /**
     * @brief Returns nominal values calculated in the last invocation of Observable::calculate method.
     */
    [[nodiscard]] virtual std::vector<std::string> getNominalValues() const = 0;

    /**
     * @brief Returns yhe naim name of the observable.
     */
    [[nodiscard]] virtual std::string getName() const = 0;
};


#endif //RAMPACK_OBSERVABLE_H
