//
// Created by Codex on 11/04/2026.
//

#include "ShapeMatcherCommon.h"

#include "core/ShapeGeometry.h"
#include "utils/Utils.h"


using namespace pyon::matcher;


MatcherArray ShapeMatcherCommon::create_vector_matcher() {
    return MatcherArray(MatcherFloat{}.mapTo<double>(), 3).mapToVector<3>();
}

MatcherArray ShapeMatcherCommon::create_axis_matcher() {
    return MatcherArray(MatcherFloat{}.mapTo<double>(), 3)
        .filter([](const ArrayData &data) {
            return std::any_of(data.begin(), data.end(), [](const Any &comp) {
                return std::abs(comp.as<double>()) > 1e-10;
            });
        })
        .describe("non-zero norm")
        .mapTo([](const ArrayData &data) {
            return data.asVector<3>().normalized();
        });
}

MatcherDictionary ShapeMatcherCommon::create_named_points_matcher() {
    auto vector = ShapeMatcherCommon::create_vector_matcher();
    return MatcherDictionary{}.valuesMatch(vector)
        .mapTo([](const DictionaryData &dict) -> ShapeGeometry::NamedPoints {
            auto map = dict.asStdMap<Vector<3>>();
            ShapeGeometry::NamedPoints points(map.begin(), map.end());
            return points;
        });
}

bool ShapeMatcherCommon::validate_axes(const DataclassData &dataclass) {
    if (dataclass["primary_axis"].isEmpty())
        return dataclass["secondary_axis"].isEmpty();
    if (!dataclass["secondary_axis"].isEmpty()) {
        auto primaryAxis = dataclass["primary_axis"].as<Vector<3>>();
        auto secondaryAxis = dataclass["secondary_axis"].as<Vector<3>>();
        return primaryAxis * secondaryAxis < 1e-12;
    }

    return true;
}
