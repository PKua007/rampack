//
// Created by Piotr Kubala on 12/02/2024.
//

#include <sstream>
#include <iomanip>
#include <limits>

#include "ShapeDataSerializer.h"
#include "utils/Exceptions.h"


ShapeDataSerializer::ParamAccess &ShapeDataSerializer::ParamAccess::operator=(const std::string &str) {
    this->paramValue = str;
    return *this;
}

ShapeDataSerializer::ParamAccess &ShapeDataSerializer::ParamAccess::operator=(const Vector<3> &v) {
    std::ostringstream ostr;
    ostr << std::setprecision(std::numeric_limits<double>::max_digits10);
    ostr << v[0] << "," << v[1] << "," << v[2];
    this->paramValue = ostr.str();
    return *this;
}
