//
// Created by Piotr Kubala on 18/02/2024.
//

#include "ShapeDataManager.h"


TextualShapeData ShapeDataManager::defaultSerialize(const ShapeData &data) const {
    TextualShapeData serializedData = this->serialize(data);
    for (auto it = serializedData.begin(); it != serializedData.end(); ) {
        const auto &paramName = it->first;
        auto defaultIt = this->defaultData.find(paramName);
        if (defaultIt != this->defaultData.end() && defaultIt->second == it->second)
            it = serializedData.erase(it);
        else
            it++;
    }

    return serializedData;
}

ShapeData ShapeDataManager::defaultDeserialize(const TextualShapeData &data) const {
    auto mergedData = data;
    auto defaultDataCopy = this->defaultData;
    mergedData.merge(std::move(defaultDataCopy));

    return this->deserialize(mergedData);
}
