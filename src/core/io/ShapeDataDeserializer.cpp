//
// Created by Piotr Kubala on 12/02/2024.
//

#include "ShapeDataDeserializer.h"
#include "utils/Exceptions.h"


ShapeDataDeserializer::ShapeDataDeserializer(const TextualShapeData &data) {
    auto hintIt = this->textualShapeData.begin();
    for (const auto &[key, value] : data)
        hintIt = this->textualShapeData.emplace_hint(hintIt, key, GuardedEntry{value});
}

Vector<3> ShapeDataDeserializer::asVector(const std::string &paramKey, const std::string &paramValue) {
    std::vector<std::string> exploded = explode(paramValue, ',');
    if (exploded.size() != 3) {
        throw ShapeDataSerializationException("Param " + paramKey
                                              + " must be a vector with 3 comma-separated entries");
    }

    Vector<3> parsed;
    std::transform(exploded.begin(), exploded.end(), parsed.begin(), [&paramKey](const auto &coordStr) {
        return ShapeDataDeserializer::asFloatingPoint<double>(paramKey, coordStr);
    });

    return parsed;
}

bool ShapeDataDeserializer::wasAccessed(const std::string &paramKey) const {
    auto it = this->textualShapeData.find(paramKey);
    Expects(it != this->textualShapeData.end());

    return it->second.wasAccessed;
}

void ShapeDataDeserializer::throwIfNotAccessed() const {
    std::vector<std::string> notAccessed;
    for (const auto &[key, entry] : this->textualShapeData)
        if (!entry.wasAccessed)
            notAccessed.push_back(key);

    if (notAccessed.empty())
        return;

    std::string notAccessedStr = implode(notAccessed, ", ");
    throw ShapeDataSerializationException("Unknown params were present: " + notAccessedStr);
}
