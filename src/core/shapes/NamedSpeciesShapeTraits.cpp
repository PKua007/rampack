//
// Created by Piotr Kubala on 13/03/2024.
//

#include "NamedSpeciesShapeTraits.h"
#include "core/io/ShapeDataSerializer.h"
#include "core/io/ShapeDataDeserializer.h"
#include "utils/Utils.h"


auto NamedSpeciesShapeTraits::findByName(const std::string &name) const -> const_iterator {
    return this->speciesMap.find(name);
}

auto NamedSpeciesShapeTraits::findByData(const ShapeData &data) const -> const_iterator {
    auto it = this->speciesMap.begin();
    while (it != this->speciesMap.end() && it->second != data)
        it++;
    return it;
}

void NamedSpeciesShapeTraits::validateShapeData(const ShapeData &data) const {
    this->underlyingDataManager.validateShapeData(data);
    ShapeDataValidateMsg(this->hasSpecies(data), "ShapeData not found in species registry");
}

TextualShapeData NamedSpeciesShapeTraits::serialize(const ShapeData &data) const {
    auto it = this->findByData(data);
    ShapeDataValidateMsg(it != this->speciesMap.end(), "ShapeData not found in species registry");

    ShapeDataSerializer serializer;
    serializer["species"] = it->first;
    return serializer.toTextualShapeData();
}

ShapeData NamedSpeciesShapeTraits::deserialize(const TextualShapeData &data) const {
    ShapeDataDeserializer deserializer(data);
    auto speciesName = deserializer.as<std::string>("species");
    deserializer.throwIfNotAccessed();

    auto it = this->findByName(speciesName);
    if (it == this->speciesMap.end())
        throw ShapeDataSerializationException("Species '" + speciesName + "' not found in the registry");

    return it->second;
}

NamedSpeciesShapeTraits::NamedSpeciesShapeTraits(const std::shared_ptr<ShapeTraits> &underlyingTraits,
                                                 const std::string &defaultName, const ShapeData &defaultData)
        : NamedSpeciesShapeTraits(underlyingTraits)
{
    this->addSpecies(defaultName, defaultData);
    this->setDefaultSpecies(defaultName);
}

void NamedSpeciesShapeTraits::addSpecies(const std::string &name, const ShapeData &data) {
    auto containsQuotes = [](const std::string &str) { return str.find('"') != std::string::npos; };
    Expects(!containsWhitespace(name) && !containsQuotes(name));
    Expects(!this->hasSpecies(name));
    Expects(!this->hasSpecies(data));

    this->speciesMap[name] = data;
}

bool NamedSpeciesShapeTraits::hasSpecies(const std::string &name) const {
    return this->findByName(name) != this->speciesMap.end();
}

bool NamedSpeciesShapeTraits::hasSpecies(const ShapeData &data) const {
    return this->findByData(data) != this->speciesMap.end();
}

void NamedSpeciesShapeTraits::setDefaultSpecies(const std::string &name) {
    Expects(this->hasSpecies(name));
    this->setDefaultShapeData({{"species", name}});
}

ShapeData NamedSpeciesShapeTraits::shapeDataForSpecies(const std::string &name) const {
    auto it = this->findByName(name);
    Expects(it != this->speciesMap.end());
    return it->second;
}

ShapeData NamedSpeciesShapeTraits::shapeDataForDefaultSpecies() const {
    const auto &defaultShapeData = this->getDefaultShapeData();
    ExpectsMsg(!defaultShapeData.empty(), "Default species is not defined");
    return this->shapeDataForSpecies(defaultShapeData.at("species"));
}
