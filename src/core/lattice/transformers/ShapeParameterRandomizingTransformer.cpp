//
// Created by Piotr Kubala on 03/03/2024.
//

#include "ShapeParameterRandomizingTransformer.h"
#include "utils/Exceptions.h"


ShapeParameterRandomizingTransformer
    ::ShapeParameterRandomizingTransformer(std::string paramName,
                                           const std::shared_ptr<ShapeParameterRandomizer> &randomizer,
                                           std::mt19937::result_type seed)
        : paramName{std::move(paramName)}, randomizer{randomizer}, mt(seed)
{
    Expects(!this->paramName.empty());
    Expects(this->randomizer != nullptr);
}

void ShapeParameterRandomizingTransformer::transform(Lattice &lattice, const ShapeTraits &shapeTraits) const {
    const auto &dim = lattice.getDimensions();
    for (std::size_t i{}; i < dim[0]; i++)
        for (std::size_t j{}; j < dim[1]; j++)
            for (std::size_t k{}; k < dim[2]; k++)
                for (auto &shape : lattice.modifySpecificCellMolecules(i, j, k))
                    this->doTransform(shape, shapeTraits);
}

void ShapeParameterRandomizingTransformer::doTransform(Shape &shape, const ShapeTraits &traits) const {
    const auto &manager = traits.getDataManager();
    auto serializedData = manager.serialize(shape.getData());

    auto paramIt = serializedData.find(this->paramName);
    TransformerValidateMsg(paramIt != serializedData.end(),
                           "Shape parameter randomization: unknown parameter " + this->paramName);
    paramIt->second = this->randomizer->randomize(paramIt->second, this->mt);

    try {
        auto reserializedData = manager.deserialize(serializedData);
        shape.setData(reserializedData);
    } catch (const ShapeDataException &e) {
        throw TransformerException(std::string("Shape parameter randomization yielded malformed shape data: ")
                                   + e.what());
    }
}
