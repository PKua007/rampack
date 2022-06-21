//
// Created by pkua on 20.06.22.
//

#include <sstream>

#include "ParameterUpdaterFactory.h"
#include "utils/Assertions.h"
#include "utils/Utils.h"
#include "core/parameter_updaters/ConstantParameterUpdater.h"
#include "core/parameter_updaters/LinearParameterUpdater.h"
#include "core/parameter_updaters/ExponentialParameterUpdater.h"
#include "core/parameter_updaters/PiecewiseParameterUpdater.h"


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

std::unique_ptr<ParameterUpdater> ParameterUpdaterFactory::create(std::string updaterString) {
    ValidateMsg(!trim(updaterString).empty(), "Dynamic parameter cannot be empty");

    std::istringstream updaterStream(updaterString);
    std::string updaterName;
    updaterStream >> updaterName;
    ValidateMsg(updaterStream, "Malformed dynamic parameter");

    // First trying reading it as an ordinary number
    try {
        auto constantUpdater = std::make_unique<ConstantParameterUpdater>(convert_string<double>(updaterName));
        ValidateMsg(fully_parsed(updaterStream), "Unexpected tokens after parameter value");
        return constantUpdater;
    } catch (StringConversionException&) { }

    if (updaterName == "const") {
        double value{};
        updaterStream >> value;
        ValidateMsg(fully_parsed(updaterStream), "Malformed const parameter. Usage: const [value]");
        return std::make_unique<ConstantParameterUpdater>(value);
    } else if (updaterName == "linear") {
        double initialValue{}, slope{};
        updaterStream >> initialValue >> slope;
        ValidateMsg(fully_parsed(updaterStream), "Malformed linear parameter. Usage: const [initial value] [slope]; "
                                                 "value = [initial value] + slope * [cycle]");
        return std::make_unique<LinearParameterUpdater>(slope, initialValue);
    } else if (updaterName == "exp") {
        double initialValue{}, rate{};
        updaterStream >> initialValue >> rate;
        ValidateMsg(fully_parsed(updaterStream), "Malformed exp parameter. Usage: const [initial value] [rate]; "
                                                 "value = [initial value] * exp(rate * [cycle])");
        return std::make_unique<ExponentialParameterUpdater>(initialValue, rate);
    } else if (updaterName == "piecewise") {
        PiecewiseParameterUpdater::UpdaterList list;
        std::string underlyingUpdaterData;
        while (std::getline(updaterStream, underlyingUpdaterData, ',')) {
            std::istringstream underlyingUpdaterDataStream(underlyingUpdaterData);
            std::size_t cycle{};
            std::string underlyingUpdater;
            underlyingUpdaterDataStream >> cycle;
            std::getline(underlyingUpdaterDataStream, underlyingUpdater);
            ValidateMsg(!underlyingUpdaterDataStream.fail(), "Malformed piecewise parameter. Usage: piecewise "
                                                             "[cycle 1] [dynamic parameter 1] , ...; [cycle 1] should "
                                                             "be equal 0");
            try {
                list.emplace_back(cycle, ParameterUpdaterFactory::create(underlyingUpdater));
            } catch (ValidationException &validationException) {
                throw ValidationException("Malformed parameter '" + underlyingUpdater + "' within piecewise parameter: "
                                          + validationException.what());
            }
        }

        ValidateMsg(!list.empty(), "Malformed piecewise parameter. Usage: piecewise "
                                   "[cycle 1] [dynamic parameter 1] , ...");
        return std::make_unique<PiecewiseParameterUpdater>(std::move(list));
    } else {
        throw ValidationException("Unknown or malformed dynamic parameter: " + updaterName);
    }
}
