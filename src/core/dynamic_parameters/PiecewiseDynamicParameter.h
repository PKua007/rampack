//
// Created by pkua on 20.06.22.
//

#ifndef RAMPACK_PIECEWISEDYNAMICPARAMETER_H
#define RAMPACK_PIECEWISEDYNAMICPARAMETER_H

#include <memory>
#include <vector>

#include "core/DynamicParameter.h"


/**
 * @brief Dynamic parameter created by patching many dynamic parameters.
 * @details More precisely, a single dynamic parameter is used for a specified number of cycles and after passing some
 * number of cycles it is changed to next one. When a specific parameter starts is given by the cycle number in the
 * same ParameterList entry as the parameter and it is ended when the cycle from the next entry is hit. Each constituent
 * parameter believes it starts from 0 and @a totalCycles is the difference between the current and next parameter,
 * or appropriately shifter @a totalCycles from master PiecewiseDynamicParameter. For example, for such ParameterList
 * @code
 * {{0, parameter1}, {1000, parameter2}, {4000, parameter3}}
 * @endcode
 * and assuming that @a totalCycles from PiecewiseDynamicParameter::getValueForCycle() is 5000:
 * <ul>
 * <li> @a parameter1 works for cycles 0-999 (and sees them as 0-999) and its @a totalCycles is 1000
 * <li> @a parameter2 works for cycles 1000-3999 (and sees them as 0-2999) and its @a totalCycles is 3000
 * <li> @a parameter3 works for cycles 4000-... (and sees them as 0-...) and its @a totalCycles is 1000 (notice that
 *      for the last parameter last cycle is included)
 * </ul>
 */
class PiecewiseDynamicParameter : public DynamicParameter {
public:
    /**
     * @brief Parameter list as a vectors of pair of numbers representing initial cycle number and dynamic parameters.
     */
    using ParameterList = std::vector<std::pair<std::size_t, std::unique_ptr<DynamicParameter>>>;

private:
     ParameterList updaters;

public:
    explicit PiecewiseDynamicParameter(ParameterList updaters);

    [[nodiscard]] double getValueForCycle(std::size_t currentCycle, std::size_t totalCycles) const override;
};


#endif //RAMPACK_PIECEWISEDYNAMICPARAMETER_H
