//
// Created by pkua on 20.06.22.
//

#include <sstream>

#include "ParameterUpdaterFactory.h"
#include "utils/Assertions.h"
#include "utils/Utils.h"
#include "core/dynamic_parameters/ConstantDynamicParameter.h"
#include "core/dynamic_parameters/LinearDynamicParameter.h"
#include "core/dynamic_parameters/ExponentialDynamicParameter.h"
#include "core/dynamic_parameters/PiecewiseDynamicParameter.h"


namespace {
    bool fully_parsed(std::istream &in) {
        if (in.fail())
            return false;
        if (in.eof())
            return true;
        in >> std::ws;
        if (in.fail() || !in.eof())
            return false;
        return true;
    }
}

std::unique_ptr<DynamicParameter> ParameterUpdaterFactory::create(std::string updaterString) {
    ValidateMsg(!trim(updaterString).empty(), "Dynamic parameter cannot be empty");

    std::istringstream paramStream(updaterString);
    std::string paramType;
    paramStream >> paramType;
    ValidateMsg(paramStream, "Malformed dynamic parameter");

    // First trying reading it as an ordinary number
    try {
        auto constParam = std::make_unique<ConstantDynamicParameter>(convert_string<double>(paramType));
        ValidateMsg(fully_parsed(paramStream), "Unexpected tokens after parameter value");
        return constParam;
    } catch (StringConversionException&) { }

    if (paramType == "const") {
        double value{};
        paramStream >> value;
        ValidateMsg(fully_parsed(paramStream), "Malformed const parameter. Usage: const [value]");
        return std::make_unique<ConstantDynamicParameter>(value);
    } else if (paramType == "linear") {
        double initialValue{}, slope{};
        paramStream >> initialValue >> slope;
        ValidateMsg(fully_parsed(paramStream), "Malformed linear parameter. Usage: const [initial value] [slope]; "
                                                 "value = [initial value] + slope * [cycle]");
        return std::make_unique<LinearDynamicParameter>(slope, initialValue);
    } else if (paramType == "exp") {
        double initialValue{}, rate{};
        paramStream >> initialValue >> rate;
        ValidateMsg(fully_parsed(paramStream), "Malformed exp parameter. Usage: const [initial value] [rate]; "
                                                 "value = [initial value] * exp(rate * [cycle])");
        return std::make_unique<ExponentialDynamicParameter>(initialValue, rate);
    } else if (paramType == "piecewise") {
        PiecewiseDynamicParameter::ParameterList list;
        std::string underlyingParamData;
        while (std::getline(paramStream, underlyingParamData, ',')) {
            std::istringstream underlyingParamDataStream(underlyingParamData);
            std::size_t cycle{};
            std::string underlyingParam;
            underlyingParamDataStream >> cycle;
            std::getline(underlyingParamDataStream, underlyingParam);
            ValidateMsg(underlyingParamDataStream, "Malformed piecewise parameter. Usage: piecewise "
                                                   "[cycle 1] [dynamic parameter 1] , ...; [cycle 1] should "
                                                   "be equal 0");
            try {
                list.emplace_back(cycle, ParameterUpdaterFactory::create(underlyingParam));
            } catch (ValidationException &validationException) {
                throw ValidationException("Malformed parameter '" + underlyingParam + "' within piecewise parameter: "
                                          + validationException.what());
            }
        }

        ValidateMsg(!list.empty(), "Malformed piecewise parameter. Usage: piecewise "
                                   "[cycle 1] [dynamic parameter 1] , ...");
        return std::make_unique<PiecewiseDynamicParameter>(std::move(list));
    } else {
        throw ValidationException("Unknown or malformed dynamic parameter: " + paramType);
    }
}
